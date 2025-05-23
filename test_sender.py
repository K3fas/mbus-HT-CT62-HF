import serial
import time
import argparse
import threading
import binascii
import struct

# --- Argument Parser Setup ---
parser = argparse.ArgumentParser(description="Periodic serial sender & receiver (with frame timeout).")
parser.add_argument('-p', '--port', type=str, default='/dev/ttyUSB0', help='Serial port (e.g., /dev/ttyUSB0 or COM3)')
parser.add_argument('-b', '--baudrate', type=int, default=9600, help='Baud rate (e.g., 9600)')
parser.add_argument('-i', '--interval', type=float, default=2.0, help='Interval between messages in seconds')
parser.add_argument('--id', type=str, default='NODE01', help='Device ID string')
parser.add_argument('--long', '-l', action='store_true', help='Send 256-byte long messages')
parser.add_argument('--mode', type=str, default='ascii', help='Comma-separated list of modes: ascii,hex,modbus')

args = parser.parse_args()
modes = [m.strip().lower() for m in args.mode.split(',') if m.strip() in {'ascii', 'hex', 'modbus'}]

# --- Constants ---
MAX_LENGTH = 255
START_NUMBER = 1
INTER_BYTE_TIMEOUT = 0.020  # 20 ms
stop_flag = False

from datetime import datetime

def ts():
    """Return current timestamp in ISO format with milliseconds."""
    return f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]}]"


def build_message(counter):
    if args.long:
        base = f"[{args.id}] DATA [{counter:04d}] "
        padding_len = MAX_LENGTH - len(base)
        payload = base + ("X" * max(0, padding_len))
        return payload[:MAX_LENGTH]
    else:
        return f"[{args.id}] DATA [{counter:04d}]\n"

def format_ascii(frame):
    return frame.decode(errors='replace').strip()

def format_hex(frame):
    return ' '.join(f'0x{byte:02X}' for byte in frame)

def format_modbus(frame):
    if len(frame) < 4:
        return f"[MODBUS] Too short: {format_hex(frame)}"

    addr = frame[0]
    func = frame[1]
    data = frame[2:-2]
    crc = frame[-2:]
    calc_crc = modbus_crc(frame[:-2])
    crc_ok = calc_crc == struct.unpack('<H', crc)[0]

    return f"Addr: {addr}, Func: 0x{func:02X}, Data: {binascii.hexlify(data).decode()}, CRC OK: {crc_ok}"

def modbus_crc(data):
    crc = 0xFFFF
    for b in data:
        crc ^= b
        for _ in range(8):
            if (crc & 0x0001):
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc

def process_frame(frame):
    output = []
    if 'ascii' in modes:
        output.append(f"{ts()} [ASCII ] {format_ascii(frame)}")
    if 'hex' in modes:
        output.append(f"{ts()} [HEX   ] {format_hex(frame)}")
    if 'modbus' in modes:
        output.append(f"{ts()} [MODBUS] {format_modbus(frame)}")
    return '\n'.join(output)

def read_with_timeout(ser):
    """Read until there's no new data for INTER_BYTE_TIMEOUT."""
    frame = bytearray()
    last_byte_time = time.time()

    while not stop_flag:
        if ser.in_waiting:
            byte = ser.read(1)
            frame.extend(byte)
            last_byte_time = time.time()
        elif frame:
            if time.time() - last_byte_time > INTER_BYTE_TIMEOUT:
                try:
                    print(process_frame(frame))
                except Exception as e:
                    print(f"[RECV ERROR] {e}")
                frame.clear()
        else:
            time.sleep(0.001)  # Prevent busy-wait

try:
    ser = serial.Serial(args.port, args.baudrate, timeout=0.01)
    print(f"Opened serial port {args.port} at {args.baudrate} bps.")
    print(f"Output modes: {', '.join(modes)}")

    # Start serial read thread
    recv_thread = threading.Thread(target=read_with_timeout, args=(ser,))
    recv_thread.daemon = True
    recv_thread.start()

    # Main loop: periodic sending
    counter = START_NUMBER
    while True:
        message = build_message(counter)
        ser.write(message.encode('utf-8'))
        print(f"[SEND] {message.strip()}")
        counter += 1
        time.sleep(args.interval)

except serial.SerialException as e:
    print(f"[ERROR] Serial error: {e}")

except KeyboardInterrupt:
    print("\n[EXIT] Stopping script...")

finally:
    stop_flag = True
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("[CLOSED] Serial port closed.")


