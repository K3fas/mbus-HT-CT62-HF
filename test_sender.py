import serial
import time
import argparse

# --- Argument Parser Setup ---
parser = argparse.ArgumentParser(description="Periodic serial sender.")
parser.add_argument('-p', '--port', type=str, default='/dev/ttyUSB0', help='Serial port (e.g., /dev/ttyUSB0 or COM3)')
parser.add_argument('-b', '--baudrate', type=int, default=9600, help='Baud rate (e.g., 9600)')
parser.add_argument('-i', '--interval', type=float, default=2.0, help='Interval between messages in seconds')
parser.add_argument('--id', type=str, default='NODE01', help='Device ID string')

args = parser.parse_args()

# --- Constants ---
MESSAGE_TEMPLATE = "[{id}] DATA [{num:04d}]\n"
START_NUMBER = 1

try:
    ser = serial.Serial(args.port, args.baudrate, timeout=1)
    print(f"Opened serial port {args.port} at {args.baudrate} bps.")

    counter = START_NUMBER
    while True:
        message = MESSAGE_TEMPLATE.format(id=args.id, num=counter)
        ser.write(message.encode('utf-8'))
        print(f"[{time.strftime('%H:%M:%S')}] Sent: {message.strip()}")
        counter += 1
        time.sleep(args.interval)

except serial.SerialException as e:
    print(f"Serial error: {e}")

except KeyboardInterrupt:
    print("\nExiting script.")

finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Serial port closed.")

