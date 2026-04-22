import serial
import csv
import time
import os
from datetime import datetime

PORT = "/dev/cu.usbmodemF412FA9F4F502" # ls /dev/cu.usb*
BAUD = 9600
OUTPUT_FILE = "hamster_data.csv"

def parse_line(line):
    try:
        parts = {}
        segments = line.split(", ")
        for seg in segments:
            if "RPM" in seg:
                parts["rpm"] = float(seg.split(":")[1].strip())
            elif "count" in seg:
                parts["count"] = int(seg.split("=")[1].strip())
            elif "distance" in seg:
                parts["distance"] = float(seg.split(":")[1].strip())
        return parts if len(parts) == 3 else None
    except:
        return None

file_exists = os.path.exists(OUTPUT_FILE)    # check if the file already exist
csvfile = open(OUTPUT_FILE, "a", newline="") # "a": append - new data added to the end
writer = csv.writer(csvfile)

if not file_exists: # if brand new file, write header
    writer.writerow(["timestamp", "rpm", "count", "distance_m"])

ser = serial.Serial(PORT, BAUD, timeout=2)
time.sleep(2)
print("Logging... press Ctrl+C to stop")

try:
    while True:
         # read line from serial data then translate bytes to human text
        raw = ser.readline().decode("utf-8").strip()
        data = parse_line(raw)
        if data:
            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            writer.writerow([timestamp, data["rpm"], data["count"], data["distance"]])
            # force the computer to write data to hard drive immediately in case lose data if program crash
            csvfile.flush()

except KeyboardInterrupt:
    print("Stopped.")
finally:
    csvfile.close()
    ser.close()