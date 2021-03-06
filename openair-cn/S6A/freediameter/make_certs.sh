#!/bin/bash
################################################################################
#   OpenAirInterface
#   Copyright(c) 1999 - 2014 Eurecom
#
#    OpenAirInterface is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#
#    OpenAirInterface is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with OpenAirInterface.The full GNU General Public License is
#    included in this distribution in the file called "COPYING". If not,
#    see <http://www.gnu.org/licenses/>.
#
#  Contact Information
#  OpenAirInterface Admin: openair_admin@eurecom.frARG is
#  OpenAirInterface Tech : openair_tech@eurecom.fr
#  OpenAirInterface Dev  : openair4g-devel@eurecom.fr
#
#  Address      : Eurecom, Compus SophiaTech 450, route des chappes, 06451 Biot, France.
#
################################################################################

# ARG is REALM
# BY DEFAULT REALM IS "eur"

DEFAULTREALMVALUE="eur"
REALM=${1:-$DEFAULTREALMVALUE}

rm -rf demoCA
mkdir demoCA
echo 01 > demoCA/serial
touch demoCA/index.txt

user=$(whoami)
HOSTNAME=$(hostname -f)

echo "Creating MME certificate for user '$HOSTNAME'.'$REALM'"

# CA self certificate
openssl req  -new -batch -x509 -days 3650 -nodes -newkey rsa:1024 -out mme.cacert.pem -keyout mme.cakey.pem -subj /CN=$REALM/C=FR/ST=PACA/L=Aix/O=Eurecom/OU=CM

openssl genrsa -out mme.key.pem 1024
openssl req -new -batch -out mme.csr.pem -key mme.key.pem -subj /CN=$HOSTNAME.$REALM/C=FR/ST=PACA/L=Aix/O=Eurecom/OU=CM
openssl ca -cert mme.cacert.pem -keyfile mme.cakey.pem -in mme.csr.pem -out mme.cert.pem -outdir . -batch

if [ ! -d /usr/local/etc/freeDiameter ]
then
    echo "Creating non existing directory: /usr/local/etc/freeDiameter/"
    sudo mkdir /usr/local/etc/freeDiameter/
fi

sudo cp -uv mme.key.pem mme.cert.pem mme.cacert.pem mme.cakey.pem /usr/local/etc/freeDiameter/

# openssl genrsa -out ubuntu.key.pem 1024
# openssl req -new -batch -x509 -out ubuntu.csr.pem -key ubuntu.key.pem -subj /CN=ubuntu.localdomain/C=FR/ST=BdR/L=Aix/O=fD/OU=Tests
# openssl ca -cert cacert.pem -keyfile cakey.pem -in ubuntu.csr.pem -out ubuntu.cert.pem -outdir . -batch
