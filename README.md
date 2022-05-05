# SIMULATORE DI RETE NS3 CON PROTOCOLLO DASH

## INSTALLAZIONE

### INSTALLAZIONE NS3
Seguire le istruzioni contenute nel tutorial contenuto in questa pagina https://www.nsnam.org/documentation/ che spiegano come installare ns3 (la repository github di riferimento è: https://github.com/nsnam/ns-3-dev-git). La libreria è stata testata su linux, è quindi preferibile scegliere l'installazione per linux. Il percorso di installazione utilizzato è quello tramite bake. La versione utilizzata ai fini della realizzazione di questa repository è ns 3-35.

### DOWNLOAD NS3-DASH
A questo punto è possibile scaricare la seguente repository all'interno della cartella src di ns3 generata al passo precedente (N.B. rinominare la cartella contente il codice da "dash-ns3" a "dash"). Se si è installato ns3 con bake il percorso sarà bake/source/ns3.35/src. Spostare lo script python "log_producer.py" in una cartella chiamta files, dove si trova la cartella non è importante.

## ESECUZIONE SIMULAZIONE
Adesso è possibile lanciare la simulazione tramite i seguenti comandi (se si è installato ns3 tramite bake):

    cd /bake/source/ns-3.35/
    ./waf configure --enable-examples --enable-test
    ./waf --run 'src/dash/examples/dash-example' > 	output.txt

N.B.: il nome del file di uscita non va cambiato in quanto sarà fornito in ingresso ad uno script python, utilizzato per ottenere il log finale

### PARAMETRI SIMULAZIONE

 - 200 ue 
 - 20 enb
 - 3600 secondi simulazione (1 ora)
 - 10000 m^2 spazio degli ue per muoversi
 - 1 m/s velocità massima degli ue

I primi 3 parametri sono modificabili passandoli come comandi da terminale, mentre gli ultimi due devono essere cambiati nel codice dove sono presenti i commenti "//HERE YOU CAN CHANGE...", nelle righe 121 e 123. Se si sceglie di modificare questi il quarto parametro (spazio di movimento) potrebbe essere necessario modificare l'istanziazione di positionAlloc e positionAlloc2, in quanto potrebbero collocare gli ue e gli enb fuori dal nuovo perimetro.

il comando con i parametri passati da terminale (con ad esempio 2 ue, 2 enb, 10sec stop time):
    cd /bake/source/ns-3.35/
    ./waf configure --enable-examples --enable-test
    ./waf --run 'src/dash/examples/dash-example' --numUeNodes=2 --numEnbNodes=2 --stopTime=10.0> 	output.txt
    
## GENERAZIONE DEI LOG
Terminata l'esecuzione è necessario spostare o copiare i seguenti files generati nella simulazione: "output.txt", "DlMacStats.txt", "DlPdcpStats.txt", "DlMacStats.txt", all'interno della cartella files precedentemente creata. A questo punto si deve lanciare lo script python "log_producer.py" dall'interno della cartella files, al termine dell'esecuzione la cartella conterrà due file csv contenenti i log relativi al segmento ("log_segmento.csv") e quelli catturati ogni 0.5 secondi("log_05").

	
