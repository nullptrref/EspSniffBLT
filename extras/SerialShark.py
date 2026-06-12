# Made by @xdavidhu (github.com/xdavidhu, https://xdavidhu.me/)

import serial
import io
import os
import subprocess
import sys
import time

import serial.tools.list_ports
import serial.serialutil
from serial.tools.list_ports_common import ListPortInfo

VALID_VIDS = [ 6790 ]


def main() -> None:
    wireshark = sys.argv[1]
    try:
        ports: list[ListPortInfo] = [port for port in serial.tools.list_ports.comports() if port.vid in VALID_VIDS]
        if len(ports) == 1:
            print(f"[*] Detected compatible serial port: {ports[0].device} - {ports[0].description}")
            serialport = ports[0].device
        elif len(ports) > 1:
            print()
            [print(f"\t<{i}> | {port.device} - {port.usb_info()}")
             for i, port in enumerate(ports, 1)]
            canBreak = False
            while not canBreak:
                try:
                    serialport = ports[int(
                        input("[?] Select a serial port:"))].device
                    canBreak = True
                except KeyboardInterrupt:
                    print("\n[+] Exiting...")
                    exit()
                except Exception as e:
                    print("[!] Please enter a number!")
        else:
            print("[!] No serial port found.")
            print("\n[+] Exiting...")
            exit()
            
        canBreak = False
        while not canBreak:
            boardRateInput = input(
                "[?] Select a baudrate (default '921600'): ")
            if boardRateInput == "":
                boardRate = 921600
                canBreak = True
            else:
                try:
                    boardRate = int(boardRateInput)
                except KeyboardInterrupt:
                    print("\n[+] Exiting...")
                    exit()
                except Exception as e:
                    print("[!] Please enter a number!")
                    continue
                canBreak = True
    except KeyboardInterrupt:
        print("\n[+] Exiting...")
        exit()

    canBreak = False
    maxRetries = 3
    failCount = 0
    while not canBreak:
        try:
            ser = serial.Serial(serialport, boardRate)
            canBreak = True
        except KeyboardInterrupt:
            print("\n[+] Exiting...")
            exit()
        except:
            print("[!] Serial connection failed... Retrying...")
            time.sleep(2)
            failCount += 1
            if failCount >= maxRetries:
                print(f"[!] Failed to connect to serial port after {maxRetries} tries.")
                print("\n[+] Exiting...")
                exit()
            continue

    print("[+] Serial connected. Name: " + ser.name)

    check = 0
    while check == 0:
        try:
            line = ser.readline()
            if b"<<START>>" in line:
                check = 1
                print("[+] Stream started...")
            # else: print '"'+line+'"'
        except serial.serialutil.SerialException:
            print("[!] Serial connection closed!")
            print("\n[+] Exiting...")
            exit()

    print("[+] Starting up wireshark...")
    cmd = f"{wireshark} -k -i -"
    p = subprocess.Popen(cmd, stdin=subprocess.PIPE, shell=True)

    try:
        while True:
            p.stdin.write(ser.read_all())
            p.stdin.flush()
    except KeyboardInterrupt:
        print("[+] Stopping...")
    except serial.serialutil.SerialException:
        print("[!] Serial connection closed!")
        print("[+] Stopping...")

    ser.close()
    print("[+] Done.")


if __name__ == "__main__" and len(sys.argv) > 1:
    main()