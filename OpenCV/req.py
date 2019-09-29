import requests
import sys

data = open('frame.jpg', 'rb').read()
url = 'http://hack.ksu.ru.com:100/php/api/person/create.php?login='+sys.argv[1]+'&status='+sys.argv[2]+'&countMove='+sys.argv[3]+'&pulse='+sys.argv[4]+'&location='+sys.argv[5]+'&creationDate='+sys.argv[6]       
res = requests.post(url, data=data, headers={'Content-Type': 'application/x-binary'})
print('Request:\n'+url)
if res.text is '':
    print('Answer: All Ok!')
else:
    print('Answer:',res.text)