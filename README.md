# SIMULATORE DI RETE NS3 CON PROTOCOLLO DASH

## INSTALLAZIONE

### INSTALLAZIONE NS3
Seguire le istruzioni contenute nel tutorial contenuto in questa pagina https://www.nsnam.org/documentation/ che spiegano come installare ns3 (la repository github di riferimento è: https://github.com/nsnam/ns-3-dev-git). La libreria è stata testata su linux, è quindi preferibile scegliere l'installazione per linux. Il percorso di installazione utilizzato è quello tramite bake. La versione utilizzata ai fini della realizzazione di questa repository è ns 3-35.

### DOWNLOAD NS3-DASH
A questo punto è possibile scaricare la seguente repository all'interno della cartella src di ns3 generata al passo precedente (N.B. rinominare la cartella contente il codice da "dash-ns3" a "dash"). Se si è installato ns3 con bake il percorso sarà bake/source/ns3.35/src.

## ESECUZIONE SIMULAZIONE
Adesso è possibile lanciare la simulazione tramite i seguenti comandi:

    cd /bake/source/ns-3.35/
    ./waf configure --enable-examples --enable-test
    ./waf --run 'src/dash/examples/dash-example' > 	output.txt

N.B.: il nome del file di uscita non va cambiato in quanto sarà fornito in ingresso ad uno script python, utilizzato per ottenere il log finale

### PARAMETRI SIMULAZIONE

 - 200 ue 
 - 20 enb
 - 10000 m^2 spazio degli ue per muoversi
 - 1 m/s velocità massima degli ue
 - 3600 secondi simulazione (1 ora)

## MODIFICA DEL CODICE
É possibile modificare il codice al fine di testarlo con diversi parametri di input, a tal proposito nel codice sono segnalati dei punti con il commento "MODIFY HERE" laddove è necessario intervenire
		
	