import asyncio
import tkinter as tk
from tkinter import ttk
from bleak import BleakClient, BleakScanner
import threading
import time

# BLE UUIDs - must match ESP32
SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
THROTTLE_CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
ROTATION_CHAR_UUID = "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"

class RobotControlApp:
    def __init__(self):
        # Create main window
        self.root = tk.Tk()
        self.root.title("ESP32 Robot Test Controller")
        self.root.configure(bg='#f0f0f0')
        
        # BLE variables
        self.client = None
        self.connected = False
        self.device_address = None
        
        # Control values
        self.throttle = 0
        self.rotation = 0
        self.last_throttle = 0
        self.last_rotation = 0

        # Key states
        self.keys_pressed = {
            'w': False,
            'a': False,
            's': False,
            'd': False
        }
        
        # Control settings
        self.speed = 100
        
        # Command sending
        self.send_interval = 50  # 20Hz - less aggressive
        self.last_send_time = 0
        self.send_lock = threading.Lock()
        self.is_sending = False
        
        self.create_widgets()
        
        # Bind keyboard events
        self.root.bind('<KeyPress>', self.on_key_press)
        self.root.bind('<KeyRelease>', self.on_key_release)
        
        # Start async loop
        self.loop = asyncio.new_event_loop()
        self.thread = threading.Thread(target=self.start_loop, daemon=True)
        self.thread.start()
        
        # Start control update loop
        self.update_controls()
        
    def start_loop(self):
        asyncio.set_event_loop(self.loop)
        self.loop.run_forever()
        
    def create_widgets(self):
        # Title
        title = tk.Label(
            self.root, 
            text="ESP32 Robot - Dev Controller",
            font=('Arial', 14, 'bold'),
            bg='#f0f0f0'
        )
        title.pack(pady=10)
        
        # Connection Frame
        conn_frame = tk.LabelFrame(self.root, text="Connection", padx=10, pady=10, bg='#f0f0f0')
        conn_frame.pack(pady=5, padx=10, fill="x")
        
        self.status_label = tk.Label(
            conn_frame,
            text="Status: Disconnected",
            font=('Arial', 10),
            bg='#f0f0f0',
            fg='red'
        )
        self.status_label.pack()
        
        # Buttons
        btn_frame = tk.Frame(conn_frame, bg='#f0f0f0')
        btn_frame.pack(pady=5)
        
        self.scan_button = tk.Button(
            btn_frame,
            text="Scan",
            command=self.scan_devices,
            width=10
        )
        self.scan_button.pack(side="left", padx=2)
        
        self.connect_button = tk.Button(
            btn_frame,
            text="Connect",
            command=self.connect_device,
            state="disabled",
            width=10
        )
        self.connect_button.pack(side="left", padx=2)
        
        self.disconnect_button = tk.Button(
            btn_frame,
            text="Disconnect",
            command=self.disconnect_device,
            state="disabled",
            width=10
        )
        self.disconnect_button.pack(side="left", padx=2)
        
        # Device dropdown
        self.device_var = tk.StringVar(value="No devices")
        self.device_menu = ttk.Combobox(
            conn_frame,
            textvariable=self.device_var,
            state="disabled",
            width=30
        )
        self.device_menu.pack(pady=5)
        
        # Controls Frame
        control_frame = tk.LabelFrame(self.root, text="WASD Controls", padx=10, pady=10, bg='#f0f0f0')
        control_frame.pack(pady=5, padx=10, fill="both", expand=True)
        
        # Keys display
        keys_frame = tk.Frame(control_frame, bg='#f0f0f0')
        keys_frame.pack(pady=10)
        
        # W
        w_frame = tk.Frame(keys_frame, bg='#f0f0f0')
        w_frame.grid(row=0, column=1, padx=3, pady=3)
        self.w_key = tk.Label(
            w_frame,
            text="W",
            width=4,
            height=2,
            font=('Courier', 12, 'bold'),
            bg='white',
            relief='raised',
            borderwidth=2
        )
        self.w_key.pack()
        tk.Label(w_frame, text="Fwd", font=('Arial', 8), bg='#f0f0f0').pack()
        
        # A
        a_frame = tk.Frame(keys_frame, bg='#f0f0f0')
        a_frame.grid(row=1, column=0, padx=3, pady=3)
        self.a_key = tk.Label(
            a_frame,
            text="A",
            width=4,
            height=2,
            font=('Courier', 12, 'bold'),
            bg='white',
            relief='raised',
            borderwidth=2
        )
        self.a_key.pack()
        tk.Label(a_frame, text="Left", font=('Arial', 8), bg='#f0f0f0').pack()
        
        # S
        s_frame = tk.Frame(keys_frame, bg='#f0f0f0')
        s_frame.grid(row=1, column=1, padx=3, pady=3)
        self.s_key = tk.Label(
            s_frame,
            text="S",
            width=4,
            height=2,
            font=('Courier', 12, 'bold'),
            bg='white',
            relief='raised',
            borderwidth=2
        )
        self.s_key.pack()
        tk.Label(s_frame, text="Back", font=('Arial', 8), bg='#f0f0f0').pack()
        
        # D
        d_frame = tk.Frame(keys_frame, bg='#f0f0f0')
        d_frame.grid(row=1, column=2, padx=3, pady=3)
        self.d_key = tk.Label(
            d_frame,
            text="D",
            width=4,
            height=2,
            font=('Courier', 12, 'bold'),
            bg='white',
            relief='raised',
            borderwidth=2
        )
        self.d_key.pack()
        tk.Label(d_frame, text="Right", font=('Arial', 8), bg='#f0f0f0').pack()
        
        self.key_labels = {
            'w': self.w_key,
            'a': self.a_key,
            's': self.s_key,
            'd': self.d_key
        }
        
        # Speed control
        speed_frame = tk.Frame(control_frame, bg='#f0f0f0')
        speed_frame.pack(pady=10, fill="x")
        
        tk.Label(
            speed_frame,
            text="Speed:",
            font=('Arial', 9),
            bg='#f0f0f0'
        ).pack(side="left")
        
        self.speed_slider = tk.Scale(
            speed_frame,
            from_=0,
            to=100,
            orient='horizontal',
            command=self.update_speed,
            bg='#f0f0f0'
        )
        self.speed_slider.set(100)
        self.speed_slider.pack(side="left", fill="x", expand=True, padx=5)
        
        self.speed_label = tk.Label(
            speed_frame,
            text="100%",
            font=('Arial', 9, 'bold'),
            bg='#f0f0f0',
            width=5
        )
        self.speed_label.pack(side="left")
        
        # Values display
        values_frame = tk.Frame(control_frame, bg='#f0f0f0')
        values_frame.pack(pady=10, fill="x")
        
        self.throttle_label = tk.Label(
            values_frame,
            text="Throttle: 0",
            font=('Courier', 10),
            bg='#f0f0f0'
        )
        self.throttle_label.pack(side="left", padx=10, expand=True)
        
        self.rotation_label = tk.Label(
            values_frame,
            text="Rotation: 0",
            font=('Courier', 10),
            bg='#f0f0f0'
        )
        self.rotation_label.pack(side="right", padx=10, expand=True)
        
        # Stop button
        self.stop_button = tk.Button(
            self.root,
            text="STOP (Space)",
            command=self.emergency_stop,
            bg='red',
            fg='white',
            font=('Arial', 10, 'bold'),
            height=2
        )
        self.stop_button.pack(pady=10, padx=10, fill="x")
        
        # Debug log
        log_frame = tk.LabelFrame(self.root, text="Log", padx=5, pady=5, bg='#f0f0f0')
        log_frame.pack(pady=5, padx=10, fill="x")
        
        self.log_text = tk.Text(log_frame, height=4, font=('Courier', 8), bg='white')
        self.log_text.pack(fill="x")
        self.log("Ready")
        
    def log(self, message):
        self.log_text.insert('end', f"{message}\n")
        self.log_text.see('end')
        if self.log_text.index('end-1c').split('.')[0] != '1':
            lines = int(self.log_text.index('end-1c').split('.')[0])
            if lines > 50:
                self.log_text.delete('1.0', '2.0')
        
    def update_speed(self, value):
        self.speed = int(float(value))
        self.speed_label.configure(text=f"{self.speed}%")
        
    def on_key_press(self, event):
        key = event.keysym.lower()

        if key == 'space':
            self.emergency_stop()
            return

        if key in self.keys_pressed:
            # Ignore repeat events - tkinter sends multiple KeyPress events when key is held
            if self.keys_pressed[key]:
                return
            self.keys_pressed[key] = True
            self.key_labels[key].configure(bg='lightblue', relief='sunken')
                
    def on_key_release(self, event):
        key = event.keysym.lower()
        
        if key in self.keys_pressed:
            self.keys_pressed[key] = False
            self.key_labels[key].configure(bg='white', relief='raised')
            
    def update_controls(self):
        new_throttle = 0
        new_rotation = 0

        if self.keys_pressed['w']:
            new_throttle += self.speed
        if self.keys_pressed['s']:
            new_throttle -= self.speed
        if self.keys_pressed['a']:
            new_rotation -= self.speed
        if self.keys_pressed['d']:
            new_rotation += self.speed

        new_throttle = max(-100, min(100, new_throttle))
        new_rotation = max(-100, min(100, new_rotation))

        self.throttle = new_throttle
        self.rotation = new_rotation
        self.update_display()

        # Only send commands if connected and values have changed
        if self.connected:
            current_time = time.time() * 1000
            has_changed = (self.throttle != self.last_throttle or
                          self.rotation != self.last_rotation)

            # Send if values changed OR if it's been too long (keep-alive)
            time_elapsed = current_time - self.last_send_time >= self.send_interval

            if has_changed or (time_elapsed and not self.is_sending):
                with self.send_lock:
                    if not self.is_sending:
                        self.is_sending = True
                        self.last_send_time = current_time
                        self.last_throttle = self.throttle
                        self.last_rotation = self.rotation
                        asyncio.run_coroutine_threadsafe(self.send_controls(), self.loop)

        self.root.after(10, self.update_controls)
        
    def update_display(self):
        self.throttle_label.configure(text=f"Throttle: {self.throttle:+4d}")
        self.rotation_label.configure(text=f"Rotation: {self.rotation:+4d}")
        
    def scan_devices(self):
        self.scan_button.configure(state="disabled", text="Scanning...")
        self.log("Scanning for devices...")
        asyncio.run_coroutine_threadsafe(self.scan_ble_devices(), self.loop)
        
    async def scan_ble_devices(self):
        try:
            devices = await BleakScanner.discover(timeout=5.0)
            robot_devices = {}
            
            for device in devices:
                if device.name and "Robot" in device.name:
                    robot_devices[device.name] = device.address
                    
            self.root.after(0, self.update_device_list, robot_devices)
            
        except Exception as e:
            self.root.after(0, self.log, f"Scan error: {str(e)}")
            self.root.after(0, lambda: self.scan_button.configure(state="normal", text="Scan"))
            
    def update_device_list(self, devices):
        if devices:
            device_list = list(devices.keys())
            self.device_menu['values'] = device_list
            self.device_menu['state'] = 'readonly'
            self.device_var.set(device_list[0])
            self.connect_button.configure(state="normal")
            self.device_addresses = devices
            self.log(f"Found {len(devices)} device(s)")
        else:
            self.device_menu['values'] = ["No robot found"]
            self.device_menu['state'] = 'disabled'
            self.device_var.set("No robot found")
            self.log("No robot devices found")
            
        self.scan_button.configure(state="normal", text="Scan")
        
    def connect_device(self):
        device_name = self.device_var.get()
        if device_name in self.device_addresses:
            self.device_address = self.device_addresses[device_name]
            self.connect_button.configure(state="disabled", text="Connecting...")
            self.log(f"Connecting to {device_name}...")
            asyncio.run_coroutine_threadsafe(self.connect_ble(), self.loop)
            
    async def connect_ble(self):
        try:
            self.client = BleakClient(self.device_address)
            await self.client.connect()
            self.connected = True
            self.root.after(0, self.on_connected)
            
        except Exception as e:
            self.root.after(0, self.log, f"Connection failed: {str(e)}")
            self.root.after(0, lambda: self.connect_button.configure(state="normal", text="Connect"))
            
    def on_connected(self):
        self.status_label.configure(text="Status: Connected", fg='green')
        self.connect_button.configure(state="disabled", text="Connected")
        self.disconnect_button.configure(state="normal")
        self.scan_button.configure(state="disabled")
        self.device_menu['state'] = 'disabled'
        self.log("Connected successfully!")

        # Reset state tracking
        self.last_send_time = time.time() * 1000
        self.last_throttle = 0
        self.last_rotation = 0
        with self.send_lock:
            self.is_sending = False
        
    def disconnect_device(self):
        self.log("Disconnecting...")
        asyncio.run_coroutine_threadsafe(self.disconnect_ble(), self.loop)
        
    async def disconnect_ble(self):
        if self.client:
            self.throttle = 0
            self.rotation = 0
            await self.send_controls()
            
            await self.client.disconnect()
            self.connected = False
            self.root.after(0, self.on_disconnected)
            
    def on_disconnected(self):
        self.status_label.configure(text="Status: Disconnected", fg='red')
        self.connect_button.configure(state="normal", text="Connect")
        self.disconnect_button.configure(state="disabled")
        self.scan_button.configure(state="normal")
        self.device_menu['state'] = 'readonly'
        self.log("Disconnected")
        
    async def send_controls(self):
        if self.connected and self.client:
            try:
                # Capture values at time of sending to avoid race conditions
                throttle_val = self.throttle
                rotation_val = self.rotation

                await self.client.write_gatt_char(
                    THROTTLE_CHAR_UUID,
                    str(throttle_val).encode()
                )
                await self.client.write_gatt_char(
                    ROTATION_CHAR_UUID,
                    str(rotation_val).encode()
                )
            except Exception as e:
                self.root.after(0, self.log, f"Send error: {e}")
            finally:
                # Release the lock so next send can proceed
                with self.send_lock:
                    self.is_sending = False
                
    def emergency_stop(self):
        for key in self.keys_pressed:
            self.keys_pressed[key] = False
            self.key_labels[key].configure(bg='white', relief='raised')

        self.throttle = 0
        self.rotation = 0
        self.update_display()
        self.log("EMERGENCY STOP")

        if self.connected:
            # Force send by resetting last values
            self.last_throttle = 999
            self.last_rotation = 999
            with self.send_lock:
                if not self.is_sending:
                    self.is_sending = True
                    self.last_throttle = 0
                    self.last_rotation = 0
                    asyncio.run_coroutine_threadsafe(self.send_controls(), self.loop)
        
    def run(self):
        self.root.mainloop()
        self.loop.call_soon_threadsafe(self.loop.stop)

if __name__ == "__main__":
    app = RobotControlApp()
    app.run()