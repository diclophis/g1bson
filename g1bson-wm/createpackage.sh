# $Id: createpackage.sh,v 1.10 2006/10/24 08:10:41 whot Exp $
#
# This script creates a debian package for the MPWM.
#
#

APPNAME=mpwm
VERSION=`cvs log Changelog | grep TAG | head -n 1 | sed -e "s/TAG: MPWM_//" | sed -e "s/_/./g"`

if [ "$1" != "--skip-deb" ]
then
    rm -rf Debian
    mkdir -p Debian/opt/MPX/bin
    mkdir -p Debian/opt/MPX/share/$APPNAME/images
    mkdir -p Debian/DEBIAN/
    chmod 755 Debian/DEBIAN/
    cat ${APPNAME}-control | sed -e "s/VERSION/$VERSION/" > Debian/DEBIAN/control
    cd src
    make dist
    cd ${OLDPWD} 

    cp src/${APPNAME} Debian/opt/MPX/bin/
    cp images/* Debian/opt/MPX/share/$APPNAME/images/

    dpkg-deb --build Debian ${APPNAME}-${VERSION}-i386.deb

    rm -rf Debian
fi

# create source package
if [ "$1" != "--skip-tar" ]
then
    rm *.tar.gz
    EXCLUDE_FILE=/tmp/${APPNAME}_exclude_tarlist  

cat << END > ${EXCLUDE_FILE}
*.deb
*.sh
src/*.o
CVS
${APPNAME}-control
*.tar.gz
END

    tar zcf ${APPNAME}-${VERSION}.tar.gz --exclude-from=${EXCLUDE_FILE} ../MPGWM++

    rm ${EXCLUDE_FILE}
fi

