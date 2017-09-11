#!/usr/bin/env python
# encoding: utf-8

import logging
logging.getLogger("scapy.runtime").setLevel(logging.ERROR)
from scapy.all import *



##------------------------------------------------------------------------------
""" Funkce jsonWindow ulozi do log souboru inforamce pro analyzu window scaling """
def jsonWindow(IfpName, pkts, fpName, title, ip, epocheTime, axisX, axisY):

    outputStr = ""
    pktsCount = len(pkts)

    pktSend = 0

    for pkt in pkts:
        if pkt[IP].fields['src'] == ip:
            pktSend += 1

    try:
        fp = open(fpName, "wb")
        fp.write('{"name":[{\n')
        fp.write('"title": "' + title + '",\n')
        fp.write('"label": "Window",\n')
        fp.write('"count": "' + str(pktsCount) + ', Soubor: ' +  IfpName +', Odeslané pakety: ' + str(pktSend) +'",\n')
        fp.write('"axisX": "' + axisX + '",\n')
        fp.write('"axisY": "' + axisY + '",\n')
        fp.write('\t"time": [')

        for pkt in pkts:
            if pkt[IP].fields['src'] == ip:
                outputStr += str(pkt.time-epocheTime)+',\n'

        listS = list(outputStr)
        index = len(outputStr) - 2
        listS[index] = ""
        outputStr = "".join(listS)

        outputStr += "],\n"

        fp.write(outputStr)

        outputStr = ""
        fp.write('\t"data": [')

        for pkt in pkts:
            if pkt[IP].fields['src'] == ip:
                outputStr += str(pkt[TCP].fields['window']) +',\n'

        listS = list(outputStr)
        index = len(outputStr) - 2
        listS[index] = ""
        outputStr = "".join(listS)

        outputStr += "]}\n"


        fp.write(outputStr)
        fp.write("]}\n")


    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
    finally:
        fp.close()

##------------------------------------------------------------------------------
""" Funkce jsonSeq ulozi do log souboru inforamce pro analyzu sekvencnich cisel """
def jsonSeq(IfpName,pkts, fpName, title, ip, epocheTime, axisX, axisY):

    outputStr = ""
    pktsCount = len(pkts)
    pktSend = 0

    for pkt in pkts:
        if pkt[IP].fields['src'] == ip:
            pktSend += 1

    try:
        fp = open(fpName, "wb")
        fp.write('{"name":[{\n')
        fp.write('"title": "' + title + '",\n')
        fp.write('"label": "SeqNumber",\n')
        fp.write('"count": "' + str(pktsCount) + ', Soubor: ' +  IfpName +', Odeslané pakety: ' + str(pktSend) +'",\n')
        fp.write('"axisX": "' + axisX + '",\n')
        fp.write('"axisY": "' + axisY + '",\n')
        fp.write('\t"time": [')

        for pkt in pkts:
            if pkt[IP].fields['src'] == ip:
                outputStr += str(pkt.time-epocheTime)+',\n'

        listS = list(outputStr)
        index = len(outputStr) - 2
        listS[index] = ""
        outputStr = "".join(listS)

        outputStr += "],\n"

        fp.write(outputStr)

        outputStr = ""
        fp.write('\t"data": [')

        for pkt in pkts:
            if pkt[IP].fields['src'] == ip:
                outputStr += str(pkt[TCP].fields['seq']) +',\n'

        listS = list(outputStr)
        index = len(outputStr) - 2
        listS[index] = ""
        outputStr = "".join(listS)

        outputStr += "]}\n"


        fp.write(outputStr)
        fp.write("]}\n")


    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
    finally:
        fp.close()

##------------------------------------------------------------------------------
""" Funkce jsonSpeed ulozi do log souboru inforamce pro analyzu propustnosti """
def jsonSpeed(IfpName,pkts, fpName, title, ip, epocheTime, axisX, axisY):

    outputStr = ""
    pktsCount = len(pkts)

    konst = epocheTime
    listI = []
    listD = []
    sumaIntervalu = 0
    sumaDat = 0
    pktSend = 0

    for pkt in pkts:
        if pkt[IP].fields['src'] == ip:
            pktSend += 1

    try:
        fp = open(fpName, "wb")
        fp.write('{"name":[{\n')
        fp.write('"title": "' + title + '",\n')
        fp.write('"label": "Throughput",\n')
        fp.write('"count": "' + str(pktsCount) + ', Soubor: ' +  IfpName +', Odeslané pakety: ' + str(pktSend) +'",\n')
        fp.write('"axisX": "' + axisX + '",\n')
        fp.write('"axisY": "' + axisY + '",\n')
        fp.write('\t"time": [')

        for pkt in pkts:
            if pkt[IP].fields['src'] == ip:
                outputStr += str(pkt.time-epocheTime)+',\n'

        listS = list(outputStr)
        index = len(outputStr) - 2
        listS[index] = ""
        outputStr = "".join(listS)

        outputStr += "],\n"

        fp.write(outputStr)

        outputStr = ""

        fp.write('\t"data": [')

        for pkt in pkts:
            if pkt[IP].fields['src'] == ip:
                newInterval = pkt.time - konst
                konst = pkt.time

                listI.append(newInterval)
                listD.append(pkt[IP].fields['len'] - 40)

                sumaIntervalu = sum(listI)

                if sumaIntervalu <= 1:
                    sumaDat = sum(listD)
                    outputStr += str(sumaDat) +',\n'

                while sumaIntervalu > 1:
                    del listI[0]
                    del listD[0]
                    listI[0] = 0
                    sumaIntervalu = sum(listI)

        listS = list(outputStr)
        index = len(outputStr) - 2
        listS[index] = ""
        outputStr = "".join(listS)

        outputStr += "]}\n"


        fp.write(outputStr)
        fp.write("]}\n")


    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
    finally:
        fp.close()

##------------------------------------------------------------------------------
""" Funkce jsonRTT ulozi do log souboru inforamce pro analyzu RTT """
def jsonRTT(IfpName,pkts, fpName, title, ip, epocheTime, axisX, axisY):

    outputStr = ""
    oldFlag = ""
    oldIp = pkts[0][IP].fields['src']
    oldTime = 0

    listT = []
    listD= []
    dictP = {}

    pktSend = 0

    for pkt in pkts:
        if pkt[IP].fields['src'] == ip:
            pktSend += 1

    pktsCount = len(pkts)

    try:
        fp = open(fpName, "wb")
        fp.write('{"name":[{\n')
        fp.write('"title": "' + title + '",\n')
        fp.write('"title": "' + title + '",\n')
        fp.write('"label": "RTT",\n')
        fp.write('"count": "' + str(pktsCount) + ', Soubor: ' +  IfpName +', Odeslané pakety: ' + str(pktSend) +'",\n')
        fp.write('"axisX": "' + axisX + '",\n')
        fp.write('"axisY": "' + axisY + '",\n')
        fp.write('\t"time":')

        for pkt in pkts:
            newFlag = pkt[TCP].fields['flags']

            ## 3-way handshake
            if newFlag == 2 and oldFlag == "":
                oldFlag = newFlag
                continue
            if newFlag == 18 and oldFlag == 2:
                oldFlag = newFlag
                continue
            if newFlag == 16 and oldFlag == 18:
                oldFlag = newFlag
                continue

            if pkt[IP].fields['src'] == ip:
                if pkt.len < 60:
                    continue

                value = dictP.get(pkt[TCP].fields['ack'])
                if value != None:
                    continue

                dictP[pkt[TCP].fields['ack']] = pkt.time

            if pkt[IP].fields['src'] != ip:
                value = dictP.get(pkt[TCP].fields['seq'])
                if value != None:
                    listD.append(pkt[TCP].fields['seq'])
                    listT.append(pkt.time - value)
                    del dictP[pkt[TCP].fields['seq']]

        outputStr = str(listD) + ',\n'
        outputStr = outputStr.replace("L", "")
        fp.write(outputStr)

        outputStr = ""

        fp.write('\t"data":')
        outputStr = str(listT) + '}\n'
        fp.write(outputStr)
        fp.write("]}\n")


    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
    finally:
        fp.close()


##------------------------------------------------------------------------------
""" Funkce getStats ziska potrebne statistiky pro analyzu TCP spojeni"""
def getStats(fileName, pkts):

    ## Nacteni src a dst IP adresy z prvniho paketu
    ipS = pkts[0][IP].fields['src']
    ipR = pkts[0][IP].fields['dst']

    epocheTime = pkts[0].time

    jsonWindow(fileName,pkts,"log/WindowSenderLog.json", 'Sender'+'('+ ipS+')'+' -> Receiver'+'('+ipR+') Window Scaling', ipS, epocheTime, "Time[s]", "Window size[B]")
    jsonWindow(fileName,pkts,"log/WindowReceiverLog.json", 'Receiver'+'('+ipR+')'+' -> Sender'+'('+ipS+') Window Scaling', ipR, epocheTime, "Time[s]", "Window size[B]")

    jsonSpeed(fileName,pkts,"log/SpeedSenderLog.json","Sender"+'('+ipS+')'+' -> Receiver'+'('+ipR+') Throughput',ipS, epocheTime,"Time[s]","Average throughput [B/s]")
    jsonSpeed(fileName,pkts,"log/SpeedReceiverLog.json","Receiver"+'('+ipR+')'+' -> Sender'+'('+ipS+') Throughput',ipR, epocheTime,"Time[s]","Average throughput [B/s]")

    jsonRTT(fileName,pkts,"log/RTTSenderLog.json","Sender"+'('+ipS+')'+' -> Receiver'+'('+ipR+') RTT',ipS, epocheTime,"Number","RTT [s]")
    jsonRTT(fileName,pkts,"log/RTTRceiverLog.json","Receiver"+'('+ipR+')'+' -> Sender'+'('+ipS+') RTT',ipR, epocheTime,"Number","RTT [s]")

    jsonSeq(fileName,pkts,"log/SeqSenderLog.json","Sender"+'('+ipS+')'+' -> Receiver'+'('+ipR+') Sequence Numbers',ipS, epocheTime,"Time[s]","Number [B/s]")
    jsonSeq(fileName,pkts,"log/SeqReceiverLog.json","Receiver"+'('+ipR+')'+' -> Sender'+'('+ipS+') Sequence Numbers',ipR, epocheTime,"Time[s]","Number [B/s]")


##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
if __name__ == '__main__':

    if len(sys.argv) == 2:
        fileName = sys.argv[1]
    else:
        print "Chyba: Spatny pocet argumentu"

    try:
        pkts = rdpcap(fileName)
    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)

    getStats(fileName,pkts)

