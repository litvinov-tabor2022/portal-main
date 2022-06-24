#!/bin/bash

function start_telnet() {
  echo "Telnet to: $1"

  n=0
   until [ $n -ge 5 ]
   do
      telnet 192.168.0.1 && break
      n=$[$n+1]
      sleep 1
   done
}

echo -e "\nSearching for available devices..."

IPs=$(nmap -sn $(ip addr show | grep "state UP" -A2 | grep "inet" | awk '{print $2}') -oG - | awk '/Up$/{print $2}')

echo -e "\nDevices found:"

for IP in $IPs; do
#    echo "Scanning $IP"
    RESP=$(curl -s --connect-timeout 1 "http://$IP/status")

    STATUS=$?
    if [ $STATUS -eq 0 ]; then
        NAME=$(echo "$RESP" | jq '.device_id' -r)

        STATUS=$?
        if [ $STATUS -eq 0 ]; then
            echo -e "$NAME\t\t|\t$IP"
        fi
    fi
done

echo -e "\nChoose IP, confirm by [ENTER]:"

read IP

echo -e "\nUploading to ${IP}...\n"

curl -X POST -F "file=@$1" "http://${IP}/upload/flash"

STATUS=$?
if [ $STATUS -ne 0 ]; then
    echo -e "\nFirmware could not be flashed!\n"
    exit 1
fi

read -p "Do you want to connect to telnet?" -n 1 -r
echo    # (optional) move to a new line
if [[ $REPLY =~ ^[Yy]$ ]]
then
    start_telnet "$IP"
fi
