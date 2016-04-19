#!/bin/bash

cat << "EOF"

            __  __       _                ____      _ _ _     _
           |  \/  | __ _| | _____ _ __   / ___|___ | | (_) __| | ___ _ __
           | |\/| |/ _` | |/ / _ \ '__| | |   / _ \| | | |/ _` |/ _ \ '__|
           | |  | | (_| |   <  __/ |    | |__| (_) | | | | (_| |  __/ |
           |_|  |_|\__,_|_|\_\___|_|     \____\___/|_|_|_|\__,_|\___|_|


EOF

if [ "$1" ] ; then
    if [ "$1" = "clean" ] ; then
        echo "## 1    ## Do clean"
        set -x
        systemctl stop xpider
        systemctl disable xpider
        rm -rf /etc/systemd/system/xpider.service
        rm -rf bin
        rm -rf xpiderctl/build
        rm -rf xpider_group/bin xpider_group/build
        rm -rf xpider_facetrack/bin xpider_facetrack/build
	rm -rf xpider_circletrack/bin xpider_circletrack/build
        rm -rf xpider_joy_control/bin xpider_joy_control/build
        set +x

        echo "## 2    ## All finish"
    else
        echo "Error!"
        echo "Usage: sh $0 (empty)/clean"
    fi
else
    echo "## 1    ## Build xpiderctl and install"
    cd xpiderctl
    mkdir build
    cd build
    cmake .. && make install
    cd ../..

    echo "## 2    ## Build xpider_facetrack"
    cd xpider_facetrack
    mkdir build
    cd build
    cmake .. && make
    cd ../..

    echo "## 3    ## Build xpider_circletrack"
    cd xpider_circletrack
    mkdir build
    cd build
    cmake .. && make
    cd ../..

    echo "## 4    ## Build xpider_joy_control"
    cd xpider_joy_control
    mkdir build
    cd build
    cmake .. && make
    cd ../..

    echo "## 5    ## Build xpider_group"
    cd xpider_group
    mkdir build
    cd build
    cmake .. && make
    cd ../..

    echo "## 6    ## Set service (default is xpider_joy_control)"
    mkdir bin
    cd bin
    ln -s ../xpider_group/bin/xpider_group xpider_group
    ln -s ../xpider_facetrack/bin/xpider_facetrack xpider_facetrack
    ln -s ../xpider_circletrack/bin/xpider_circletrack xpider_circletrack
    ln -s ../xpider_joy_control/bin/xpider_joy_control xpider_joy_control

    rm -rf service.sh
    echo "#!/bin/bash" >> service.sh
    echo "" >> service.sh
    echo "#/home/root/xpider/bin/xpider_group &" >> service.sh
    echo "#/home/root/xpider/bin/xpider_facetrack &" >> service.sh
    echo "#/home/root/xpider/bin/xpider_circletrack &" >> service.sh
    echo "/usr/sbin/rfkill unblock bluetooth" >> service.sh
    echo "/home/root/xpider/bin/xpider_joy_control AC:FD:93:CB:F1:F5 &" >> service.sh
    chmod a+x service.sh

    systemctl stop xpider
    systemctl disable xpider
    rm -rf /etc/systemd/system/xpider.service

    SERVICE=/etc/systemd/system/xpider.service
    rm -rf $SERVICE
    echo "[Unit]" > $SERVICE
    echo "Description=Xpider service" >> $SERVICE
    echo "" >> $SERVICE
    echo "[Service]" >> $SERVICE
    echo "Type=forking" >> $SERVICE
    echo "ExecStart=/home/root/xpider/bin/service.sh" >> $SERVICE
    echo "Restart=always" >> $SERVICE
    echo "RestartSec=5" >> $SERVICE
    echo "" >> $SERVICE
    echo "[Install]" >> $SERVICE
    echo "WantedBy=multi-user.target" >> $SERVICE

    systemctl enable xpider 
    systemctl restart xpider

    echo "## 7    ## All finish"
fi