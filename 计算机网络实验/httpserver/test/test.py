import requests
from os.path import dirname, realpath

requests.packages.urllib3.disable_warnings()

test_dir = dirname(realpath(__file__))

# print(test_dir)

# http 301
print("*" * 30, "http-301", "*" * 30)
r = requests.get('http://10.37.129.41/index.html', allow_redirects=False, verify=False)
assert (r.status_code == 301 and r.headers['Location'] == 'https://10.37.129.41/index.html')

# https 200 OK
print("*" * 30, "https-200", "*" * 30)
r = requests.get('https://10.37.129.41/index.html', verify=False)
assert (r.status_code == 200 and open(test_dir + '/../index.html', 'rb').read() == r.content)

# http 200 OK
print("*" * 30, "http-200", "*" * 30)
r = requests.get('http://10.37.129.41/index.html', verify=False)
assert (r.status_code == 200 and open(test_dir + '/../index.html', 'rb').read() == r.content)
#
#  http 404
print("*" * 30, "http-404", "*" * 30)
r = requests.get('http://10.37.129.41/notfound.html', verify=False)
assert (r.status_code == 404)
#
# file in directory
print("*" * 30, "file in directory", "*" * 30)
r = requests.get('http://10.37.129.41/dir/index.html', verify=False)
assert (r.status_code == 200 and open(test_dir + '/../index.html', 'rb').read() == r.content)
#
# http 206
print("*" * 30, "http-206 100-200", "*" * 30)
headers = {'Range': 'bytes=100-200'}
r = requests.get('http://10.37.129.41/index.html', headers=headers, verify=False)
assert (r.status_code == 206 and open(test_dir + '/../index.html', 'rb').read()[100:201] == r.content)
#
# http 206
headers = {'Range': 'bytes=100-'}
print("*" * 30, "http-206 100-", "*" * 30)
r = requests.get('http://10.37.129.41/index.html', headers=headers, verify=False)
assert (r.status_code == 206 and open(test_dir + '/../index.html', 'rb').read()[100:] == r.content)
