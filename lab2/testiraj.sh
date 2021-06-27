#! /bin/sh

error() {
    echo $*
    exit 2
}

himage -e CnC > /dev/null 2>&1 || error "Is the IMUNES simulation started? Do \
you have multiple experiments running? Try: Experiment->Execute or cleanupAll."

# terminiraj sve postojece programe koji mozda vec rade
himage CnC killall -9 CandC.py 2> /dev/null
himage BOT1 killall -9 bot 2> /dev/null
himage Server killall -9 server 2> /dev/null

# kopiraj sve potrebne datoteke na cvorove
hcp CandC.py CnC:/
hcp bot BOT1:/
hcp server Server:/

# postavi IP adresu i port servera u CandC.py programu na dobre vrijednosti
himage CnC sed -i .bak 's/server_ip = "\([^"]*\)"/server_ip = "10.0.0.20"/' /CandC.py
himage CnC sed -i .bak 's/server_port = "\([^"]*\)"/server_port = "1234"/' /CandC.py

# pokreni sve programe u posebnim terminal prozorima
xfce4-terminal -T 'CandC.py' -e "bash -c 'himage CnC python2.7 /CandC.py';bash"
xfce4-terminal -T 'Server' -e "bash -c 'himage Server /server -p \"Prvi payload\

:Drugi\

:Treci payload\

:\"';bash"
xfce4-terminal -T 'zrtva1' -e "bash -c 'himage zrtva1 nc -kul 1111';bash"
xfce4-terminal -T 'zrtva2' -e "bash -c 'himage zrtva2 nc -kul 2222';bash"
xfce4-terminal -T 'zrtva3' -e "bash -c 'himage zrtva3 nc -kul 3333';bash"
sleep 1
xfce4-terminal -T 'bot' -e "bash -c 'himage BOT1 /bot 10.0.0.10 5555';bash"

