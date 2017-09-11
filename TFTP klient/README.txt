Seznam odevzdaných souborů:
- mytftpclient.c
- Makefile
- manual.pdf
- README.txt

Příklad spuštění:
$ make
$ ./mytftpclient
> -R -d file.txt -a 192.168.56.1,69 -s 512 -c octet -t 2

Příklad ukončení:
- ctrl + c

Popis programu:
- TFTP klient podporující rozšíření blocksize, file transfer size, timeout
- Klient je implementován jako blokující, čeká se na provedení operace
- Program funguje jako konzolová aplikace, která zpracuje vstup a na výstup vytiskne
  informace v požadovanám formátu
- Je podporován formát adresy IPv4 i IPv6
- Volitelné rozšíření Multicast není implementováno
- Nápověda se vypíše vždy po spuštění programu
- Parametry je možné mezi sebou libovolně kombinovat
- Pokud není zadán parametr -a, nastaví se adresa 127.0.0.1
- Pokud není zadán parametr -s, nastaví se blocksize 512
- Pokud není zadán parametr -t, nastaví se hodnota 1
- Pokud není zadán parametr -c, nastaví se mod octet (binary)
- V programu je implementována možnost znovu odelast požadavek např. při selhání připojení na server, 
  neodeslání DATA nebo ACK paketu. Hodnota počtu těchto opakování je nastavena na 10

Strategie při odmítnutí parametrů -s a -t:
  -s:
- Při zadávání musí parametr -s splňovat rozsah 8 - 65464, daný v RFC dokumentu
- Po zadání konkretní hodnoty se provede porovnání nejmenší hodnoty MTU z danných rozhraní a zadané honoty,
  pokud je zadaná hodnota větší než nejmenší MTU, serveru se jako blocksize option zašle hodnota nejmenšího MTU,
  uživatelská hodnota se ignoruje
- Ve funkci pro zpracování OACK paketu od serveru se provede porovnání, zda-li server akceptoval navrhovanu hodnotu blocksize,
  pokud server nepotvrdil navrhovanou hodnotu, vypíše se upozornění a dále se pokračuje s hodnotu navrhovanou serverem

  -t:
- Při zadávání musí parametr -t splňovat rozsah 1 - 255, daný v RFC dokumentu
- Ve funkci pro zpracování OACK paketu od serveru se provede porovnání, zda-li server akceptoval navrhovanu hodnotu timeout,
  pokud server nepotvrdil navrhovanou hodnotu, pokračuje se s hodnotu navrhovanou serverem
