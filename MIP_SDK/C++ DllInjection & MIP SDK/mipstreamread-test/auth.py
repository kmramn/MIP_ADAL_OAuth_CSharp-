#
# Copyright (c) Microsoft Corporation.
# All rights reserved.
#
# This code is licensed under the MIT License.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

import getopt
import sys
import json
import ssl
import urllib
import re

try:
  from urllib.parse import urlencode
except ImportError: # backward compatible for python2
  from urllib import urlencode
try:
  from urllib.request import urlopen, Request
except ImportError: # backward compatible for python2
  from urllib2 import urlopen, Request
try:
  from urllib.request import URLError
except ImportError: # backward compatible for python2
  from urllib2 import URLError

#
# This script acquires auth tokens directly via a simple http request. This is
# included only as a means to acquire auth tokens for use by the sample apps
# and is not intended for use in production code. It will only work for tenants
# that support straightforward username/password http authentication.
#
# For proper auth integration, please use Azure Active Directory Authentication
# Library (ADAL), Active Directory v2 Libraries (MSAL), or other OAuth 2.0
# libraries:
#
# https://docs.microsoft.com/en-us/azure/active-directory/develop/active-directory-authentication-libraries
# https://docs.microsoft.com/en-us/azure/active-directory/develop/active-directory-v2-libraries
#

def printUsage():
  print('auth.py -u <username> -p <password> -a <authority> -r <resource> -c <clientId>')

def main(argv):
  try:
    options, args = getopt.getopt(argv, 'hu:p:a:r:c:')
  except getopt.GetoptError:
    printUsage()
    sys.exit(-1)

  username = ''
  password = ''
  authority = ''
  resource = ''
  clientId = ''

  for option, arg in options:
    if option == '-h':
      printUsage()
      sys.exit()
    elif option == '-u':
      username = arg
    elif option == '-p':
      password = arg
    elif option == '-a':
      authority = arg
    elif option == '-r':
      resource = arg
    elif option == '-c':
      clientId = arg

  if username == '' or password == '' or authority == '' or resource == '' or clientId == '':
    printUsage()
    sys.exit(-1)

  # Add "/oauth2/token" to get endpoint URL
  if not authority.endswith('token'):
    authority = authority + '/oauth2/token'

  # Build REST call
  headers = {
    'Content-Type': 'application/x-www-form-urlencoded',
    'Accept': 'application/json'
  }

  params = {
    'resource': resource,
    'client_id': clientId,
    'grant_type': 'password',
    'username': username,
    'password': password
  }

  req = Request(
    url = authority,
    headers = headers,
    data = urlencode(params).encode("utf-8"))

  try:
    ctx = ssl.SSLContext(ssl.PROTOCOL_SSLv23)
    # allow TLS 1.2 and later
    ctx.options |= ssl.OP_NO_SSLv2
    ctx.options |= ssl.OP_NO_SSLv3
    ctx.options |= ssl.OP_NO_TLSv1
    ctx.options |= ssl.OP_NO_TLSv1_1
    f = urlopen(req, context=ctx)
    response = f.read()
    f.close()
    sys.stdout.write(json.loads(response.decode('utf-8'))['access_token'])
  except URLError as err:
    sys.stderr.write("Attempting to acquire token via direct HTTP request to {0}\n".format(authority))
    sys.stderr.write("\tClient-ID: {0}\n".format(clientId))
    sys.stderr.write("\tResource:  {0}\n".format(resource))
    sys.stderr.write("\t=== FAILURE: {0} ===\n".format(err.reason))

if __name__ == '__main__':
  main(sys.argv[1:])
