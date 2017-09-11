#!/usr/bin/env python
# encoding: utf-8

import requests
import urllib2
import os.path
from bs4 import BeautifulSoup
import urllib3
import sys
import re


## Trida popisujici popisujici potrebne parametry fora
class Forum_Params:
    ROOT_DIRECTORY = "ModelForum.cz"
    FORUM_URL = "http://modelforum.cz"
    POST = ""

##------------------------------------------------------------------------------
""" Funkce nahradi nepovolene znaky pri vytvareni nazvu adresaru (Windows) """
def ReplaceAllNonNameChars(str):

    text = str
    pattern = re.compile(r'[?|<|>|:|*|\\|/|\||"]') ## Optimalizace pro často pouzivane regexy
    text = re.sub(pattern, " ", text)

    return text

##------------------------------------------------------------------------------
""" Funkce zjisti zda je nutne aktualizovat prispevky daneho fora """
def CheckForNews(url, file_path):
    try:
        file = open(file_path, "r")
        file_content = file.read()

        last_posts_url = url + "&start=100000"

        ## Ziskani html kodu stranky
        try:
            http = urllib3.PoolManager(headers={'User-Agent' : "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36"})
            html_content = http.request('GET', last_posts_url)

        except:
            print "Chyba pripojeni na server fora\n"
            sys.exit()

        ## Vytvoeni "Soup" z nactene HTML stranky
        soup = BeautifulSoup(html_content.data)
        soup.decode('utf-8')

        posts = soup.findAll("div", {"class": "post", "id": True})   ## Prispevky

        latest_id = ""
        for post in posts:
            id = post["id"]
            latest_id = id

        if latest_id in file_content: ## Update souboru neni treba
            return 0
        else:
            return 1 ## Byly nalezeny nove prispevky

    except IOError as e:
        return -1

    finally:
        file.close()

##------------------------------------------------------------------------------
""" Funkce zpracuje nestandartni fora, vytvori adresare a ulozi jejich prispevky """
def GetNonStad(non_stand, url, path):

    ## Vytvareni adresaru podle jmena sekce + ulozeni odkazu na podsekce
    for topic in non_stand:
        links = topic.findAll("a")

        for link in links:
            try:
                ## Jmeno sekce
                text = link.get("title")
                if (text == None) or ("Re" in text):
                    continue

                text = ReplaceAllNonNameChars(text)
                ## Link na podsekci
                link = link.get("href")
                link = link[1:]

                fullpath = os.path.join(path, text)
                if not os.path.exists(fullpath): os.makedirs(fullpath)

                ZpracujForum(url + link, fullpath, "", 0)

            except:
                continue
        break

##------------------------------------------------------------------------------
""" Funkce zpracuje html tridy forumbg a vytvori adresare """
def GetForumAB(forabgs_titles, url, path):
    ## Vytvareni adresaru podle jmena sekce + ulozeni odkazu na podsekce
    for forum_title in forabgs_titles:
        try:
            ## Jmeno sekce
            text = forum_title.text
            text = ReplaceAllNonNameChars(text)

            ## Link na podsekci
            link = forum_title.get("href")
            link = link[1:]

            ## Vytvoreni adresare
            fullpath = os.path.join(path, text)
            if not os.path.exists(fullpath): os.makedirs(fullpath)

            ZpracujForum(url + link, fullpath, "", 0)

        except:
            continue

##------------------------------------------------------------------------------
""" Funkce zpracuje html tridy forumbg a vytvori adresare """
def GetForumBG(forumbg_titles, url, path):
    ## Vytvareni adresaru podle jmena sekce + ulozeni odkazu na podsekce
    for forum_title in forumbg_titles:
        try:
            ## Jmeno sekce
            text = forum_title.text
            text = ReplaceAllNonNameChars(text)

            ## Link na podsekci
            link = forum_title.get("href")
            link = link[1:]

            ## Vytvoreni adresare
            fullpath = os.path.join(path, text)
            if not os.path.exists(fullpath): os.makedirs(fullpath)

            test_filepath = os.path.join(fullpath, "posts.txt")
            if os.path.exists(test_filepath):
                update = CheckForNews(url + link, test_filepath)

                if update > 0:
                    print "UPDATE PRISPEVKU FORA: " + test_filepath
                    ZpracujForum(url + link, fullpath, "", 0)
                else:
                    continue
            else:
                ZpracujForum(url + link, fullpath, "", 0) ## Uroven s prispevky

        except:
            continue

##------------------------------------------------------------------------------
""" Funkce zpracuje hlavni stranky fora, subsekce i strnaky s prispevky """
def ZpracujForum(url, path, old_text, index):

    ## Ziskani html kodu stranky
    try:
        http = urllib3.PoolManager(headers={'User-Agent' : "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36"})
        html_content = http.request('GET', url)

    except:
        print "Chyba pripojeni na server fora\n"
        print url
        sys.exit()

    ## Nektere obrazkove prilohy maji citac zobrazeni, to je problem nebot kazde
    ## zobrazeni stranky zvysi citac a stranka je zmenena, proto je nutne toto pocitadlo odstranit
    html_page = re.sub("Zobrazeno.*?t", '', html_content.data)

    ## Vytvoeni "Soup" z nactene HTML stranky
    soup = BeautifulSoup(html_page)
    soup.decode('utf-8')

    ## Parsovani nazvu sekci pomoci knihovny BeautifulSoup
    forabgs_titles = soup.findAll("a", {"class": "forumtitle"})  ## Hlani sekce fora
    forumbg_titles = soup.findAll("a", {"class": "topictitle"})  ## Podsekce fora
    posts = soup.findAll("div", {"class": "post", "id": True})   ## Prispevky
    non_stand = soup.findAll("ul", {"class": "forums"})          ## Nestandartni fora

    ## Zpracovani stranky
    if len(non_stand) > 0:
        GetNonStad(non_stand,ForumParams.FORUM_URL, path)

    if len(forumbg_titles) > 0:
        GetForumBG(forumbg_titles, ForumParams.FORUM_URL, path)

    if len(forabgs_titles) > 0:
        GetForumAB(forabgs_titles, ForumParams.FORUM_URL, path)

    if len(posts) > 0:
        ## Nalezeni ID prispevku + autor + text prispevku
        new_text = ""

        for post in posts:
            id = post["id"]
            new_text += id + ":\n"

            autors = post.find_all("p", {"class":"author"})
            for name in autors:
                new_text += name.text + "\n"

            contents = post.find_all("div", {"class":"content"})
            for text in contents:
                new_text += text.text.strip() + "\n"

            new_text += "--------------------------------------------------------\n"

        if old_text == new_text:
            fullpath = os.path.join(path, "posts.txt")

            ## Ulozeni prispevku do souboru post.txt
            try:
                file = open(fullpath, "w")
                file.write(ForumParams.POST.encode("utf-8"))

            except IOError as e:
                sys.exit()

            finally:
                file.close()
                ForumParams.POST = ""
                return
        else:
            ForumParams.POST += new_text
            index += 15
            url +="&start=" + str(index)
            ZpracujForum(url, path, new_text, index)


##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
##------------------------------------------------------------------------------
if __name__ == '__main__':

    ForumParams = Forum_Params()

    try:
        ## Vytvoreni korenoveho adresare ModeForum.cz
        if not os.path.exists(ForumParams.ROOT_DIRECTORY): os.makedirs(ForumParams.ROOT_DIRECTORY)

    except IOError as e:
        print "I/O error({0}): {1}".format(e.errno, e.strerror)
        sys.exit()

    print "Vytvareni adresaroveho stromu modelu fora"
    ZpracujForum(ForumParams.FORUM_URL, ForumParams.ROOT_DIRECTORY, "", 0)
    print "Stahovani fora dokonceno"