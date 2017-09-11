#!/usr/bin/env python
# encoding: utf-8

import tweepy
import dicttoxml
import re
import collections
import os.path
import string
import urllib2
import sys


## Trida popisujici autorizacni parametry pro TwitterAPI
class Twitter_API_Params:
    CONSUMER_KEY =          "IC4aT2HDxhSHO5gbmJ60d40ML"
    CONSUMER_SECRET =       "lelQvupTrbxWDAEmuL0KWYqVMj4nQ3ePdD9CpFLfUfFcbMKbMs"
    ACCESS_TOKEN =          "3077009158-A6E5gMCQ9dCO1T0xtM9MDiqc8mwI82oS03CM5jy"
    ACCESS_TOKEN_SECRET =   "6yYGGOYQ3ESCpLDYIdylQZvudXGnpPjYKi40gXZW2PIJd"

    USERNAME = "gamescz" ## uzivatel od ktereho se budou stahovat tweety
    TWEET_COUNT = 50
    LAST_TWEED_ID = 1

    TXT_ID_FILE = "gamescz-last_tweed.txt"
    XML_TWEET_FILE = "gamescz-tweets.xml"
    HTML_DIR = "Stazene stranky"

##------------------------------------------------------------------------------
""" Funkce nacte stare tweety """
def GetOldXmlData():

    try:
       file_xml_to_read = open(TwitterAuth.XML_TWEET_FILE, "rb")
       old_xml = file_xml_to_read.read()

    except IOError as e:
       print "I/O error({0}): {1}".format(e.errno, e.strerror)
       sys.exit()

    finally:
        file_xml_to_read.close()

    old_xml = string.replace(old_xml, '<?xml version="1.0" encoding="UTF-8" ?>', '')
    old_xml = string.replace(old_xml, '<root>', '')
    old_xml = string.replace(old_xml, '</root>', '')

    return old_xml

##------------------------------------------------------------------------------
""" Funkce nacte a ulozi obsah webove stranky v kazdem stazenem tweetu """
def SaveWebPageContent(list_of_urls, tweet_id):

    urls = []
    urls = list_of_urls
    poradi = 1

    for url in urls:

        try:
            filename = tweet_id + " - "+ str(poradi) + ".html"
            fullpath = os.path.join(TwitterAuth.HTML_DIR, filename)

            page = urllib2.urlopen(url)
            data = page.read()

            try:
                file_html_to_write = open(fullpath, "wb")
                file_html_to_write.write(data)

            except (IOError, urllib2.HTTPError) as e:
                print "I/O error({0}): {1}".format(e.errno, e.strerror)
                sys.exit()

            finally:
                file_html_to_write.close()

            poradi += poradi

        except urllib2.HTTPError, e:
            checksLogger.error('HTTPError = ' + str(e.code))
            sys.exit()
        except urllib2.URLError, e:
            checksLogger.error('URLError = ' + str(e.reason))
            sys.exit()

##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
if __name__ == '__main__':

    TwitterAuth = Twitter_API_Params()
    user_tweets = [] ## seznam pro uchovani tweetu
    tweet_dictionary_xml = collections.OrderedDict() ## serazeny slovnik tweetu
    urls = []
    old_xml = ""
    tweets_downloaded = 0
    pocet_stazenych_stranek = 0

    ## autorizace
    auth = tweepy.OAuthHandler(TwitterAuth.CONSUMER_KEY, TwitterAuth.CONSUMER_SECRET)
    auth.set_access_token(TwitterAuth.ACCESS_TOKEN, TwitterAuth.ACCESS_TOKEN_SECRET)

    ## inicializace TwiterAPI pomoci knihovny tweepy
    api = tweepy.API(auth)

    if os.path.isfile(TwitterAuth.TXT_ID_FILE):
        try:
           file_txt_to_read = open(TwitterAuth.TXT_ID_FILE, "rb")
           TwitterAuth.LAST_TWEED_ID = long(file_txt_to_read.readline())

           ## nacteni starych tweetu
           old_xml = GetOldXmlData()

        except IOError as e:
           print "I/O error({0}): {1}".format(e.errno, e.strerror)
           sys.exit()

        finally:
            file_txt_to_read.close()

    print "Stahovani tweetu od uzivatele @" + TwitterAuth.USERNAME + "\n"

    ## ziskani poslednich N prispevku daneho uzivatele (maximalni pocet je 200 na jeden request)
    while 1:

        try:
            new_tweets = api.user_timeline(screen_name = TwitterAuth.USERNAME, count = TwitterAuth.TWEET_COUNT, since_id = TwitterAuth.LAST_TWEED_ID)
            tweed_count = len(new_tweets)
            tweets_downloaded += tweed_count

            if tweed_count == 0:
                break

            ## ulozeni stazenych tweetu
            user_tweets.extend(new_tweets)

            if TwitterAuth.LAST_TWEED_ID == 1:
                TwitterAuth.LAST_TWEED_ID = user_tweets[0].id
                break

            TwitterAuth.LAST_TWEED_ID = user_tweets[0].id

        except:
            print "Doslo k chybe pri stahovani tweetu\n"
            sys.exit()

    print "Bylo stazeno " + (str(tweets_downloaded)) + " novych tweetu\n"

    print "Stahovani obsahu webovych stranek ...\n"
    if not os.path.exists(TwitterAuth.HTML_DIR):
        os.makedirs(TwitterAuth.HTML_DIR)

    ## tvorba xml dokomentu pomoci slovniku
    for tweet in user_tweets:
        ## nalezeni URL odkazu v tweetu
        tweet_urls = tweet.entities['urls']
        for expanded_url in tweet_urls:
            urls.append(expanded_url['expanded_url'])

        ## tvorba slovniku
        tweet_dictionary_xml.update ({tweet.id_str:
                {
                    'text': tweet.text,
                    'URL' : urls,
                    'timestamp' : tweet.created_at
                }
            })

        SaveWebPageContent(urls, tweet.id_str)

        pocet_stazenych_stranek += len(tweet_urls)
        print '{0}\r'.format(pocet_stazenych_stranek),

        urls = []

    print "\nStahovani obsahu webovych stranek - dokonceno\n"

    new_xml = dicttoxml.dicttoxml(tweet_dictionary_xml)
    new_xml = string.replace(new_xml, '</root>', '')
    new_xml += old_xml + "\n</root>"

    ## ulozeni tweetu do xml souboru
    try:
        file_xml_to_write = open(TwitterAuth.XML_TWEET_FILE, "wb")
        file_txt_to_write = open(TwitterAuth.TXT_ID_FILE, "wb")

        file_xml_to_write.write(new_xml)
        file_txt_to_write.write(str(TwitterAuth.LAST_TWEED_ID))

    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
        sys.exit()

    finally:
        file_xml_to_write.close()
        file_txt_to_write.close()