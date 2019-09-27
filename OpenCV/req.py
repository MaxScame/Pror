import requests
import sys


url = 'http://hack.ksu.ru.com:100/php/api/person/create.php?login='+sys.argv[1]+'&status='+sys.argv[2]+'&countMove='+sys.argv[3]+'&location='+sys.argv[4]+'&creationDate='+sys.argv[5]       
res = requests.post(url)    
print(res.text)