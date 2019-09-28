import requests
import sys

files = {"file": open("frame.jpg", "rb")}
url = 'http://hack.ksu.ru.com:100/php/api/person/create.php?login='+sys.argv[1]+'&status='+sys.argv[2]+'&countMove='+sys.argv[3]+'&Pulse='+sys.argv[4]+'&location='+sys.argv[5]+'&creationDate='+sys.argv[6]       
res = requests.post(url, files = files)
print(res.text)