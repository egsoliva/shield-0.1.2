import serial
import openpyxl
import time

SERIAL_PORT = "COM11"
BAUD_RATE = 9600

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)

EXCEL_FILE = "gps_log.xlsx"

try:
    workbook = openpyxl.load_workbook(EXCEL_FILE)
    sheet = workbook.active
except FileNotFoundError:
    workbook = openpyxl.Workbook()
    sheet = workbook.active
    sheet.append(["Timestamp", "Latitude", "Longitude"])

print("Logging GPS data from ESP8266 to Excel...")

last_log_time = time.time()
lat, lon = None, None
sample = 0

while sample <= 30:
    try:
        line = ser.readline().decode("utf-8").strip()
        
        if line:
            print("Received:", line)

            # Extract GPS Data
            if "Latitude:" in line:
                lat = float(line.split(":")[1].strip())
            elif "Longitude:" in line:
                lon = float(line.split(":")[1].strip())

            # Log Data Every 5 Seconds When All Values Are Available
            if lat is not None and lon is not None:
                current_time = time.time()
                if current_time - last_log_time >= 5:
                    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")

                    sheet.append([timestamp, lat, lon])
                    workbook.save(EXCEL_FILE)
                    print(f"Logged: {timestamp}, {lat}, {lon}")
                    
                    last_log_time = current_time
                    sample += 1

    except KeyboardInterrupt:
        print("\nStopping logging...")
        ser.close()
        break
    except Exception as e:
        print("Error:", e)