
import socket
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import threading
import cantools
import time
import os

# Konfigurera InfluxDB
url = "http://localhost:8086"
token = "BDLTtAA9DBb-k2z_d76h86xcc-hwGN2S3Vb1_DL12RGLei9_5gue3c0C1qiVM10RI1BaGTPyjHceKq-r_WETXw=="
org = "candata"
bucket = "test1"

client = InfluxDBClient(url=url, token=token)
write_api = client.write_api(write_options=SYNCHRONOUS)


# Läs DBC-filen
def read_dbc_file(dbc_file_path):
    try:
        db = cantools.database.load_file(dbc_file_path)
        return db
    except Exception as e:
        print("Error reading DBC file:", e)
        return None


dbc_file_path = "C:/Users/domin/OneDrive/Skrivbord/Itino_inverter.dbc"  # sökvägen till DBC-fil
db = read_dbc_file(dbc_file_path)


def send_to_influxdb(data):
    # Skapa datapunkt för varje mätning
    for message in data:
        point = Point(message.name)  # Använd meddelandets namn som mätningens namn
        for signal in message.signals:
            # Lägg till signalvärde som fält
            point.field(signal.name, data[message.name][signal.name])
        write_api.write(bucket=bucket, org=org, record=point)
    print("Data skickad till InfluxDB")


def influxdb_thread():
    while True:
        # Läs data från CAN-bussen och lagra i can_data
        can_data = {}  # Tom dictionary för att hålla data som läses in

        # Lägg till logik för att läsa CAN-data och lagra i can_data

        # Om CAN-data har lästs in, tolka den och skicka till InfluxDB
        if can_data:
            send_to_influxdb(can_data)

        # Vänta i fem minuter innan nästa sändning
        time.sleep(300)  # 300 sekunder = 5 minuter


def serial_communication_thread():
    HOST = '172.16.222.140'  # IP-adress för Arduino
    PORT = 8080

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.bind((HOST, PORT))
            s.listen()
            print(f"Lyssnar på {HOST}:{PORT}...")
            conn, addr = s.accept()
            with conn:
                print('Ansluten till', addr)
                while True:
                    data = conn.recv(1024)
                    if not data:
                        break
                    print('Data mottaget:', data.decode('utf-8'))
        except OSError as e:
            print("OSError:", e)


def main():
    influxdb_thread_instance = threading.Thread(target=influxdb_thread)
    serial_communication_thread_instance = threading.Thread(target=serial_communication_thread)

    influxdb_thread_instance.start()
    serial_communication_thread_instance.start()

    influxdb_thread_instance.join()
    serial_communication_thread_instance.join()

if __name__ == "__main__":
    main()
