#!/bin/bash
regg1="^[A-Z][a-zA-Z0-9, ]*[\.$\!$\?$]"
regg2=",[]*si[]"
regg3="[]*si[],"
count=0
if [ "$#" -ne 1 ];
    then
    echo "Numar de argumente mai mare de 1"
    exit 1
fi
caracter=$1

while read -r linie
do
    rez=$(echo $linie | grep -E "$regg1" | grep -v "$regg2" | grep -v "$regg3" | grep "$caracter")
    if [ -n "$rez" ];
    then
        count=`expr $count + 1`        
    fi
done
if [ "$count" -eq 1 ];
    then
    echo -n "A fost identificata o propozitie corecta care contine caracterul $caracter"
else
    echo -n "Au fost identificate $count propozitii corecte care contin caracterul"
fi
exit 0