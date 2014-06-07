# Maintainer: Jesse McClure AKA "Trilby" <jmcclure [at] cns [dot] umass [dot] edu>
_gitname="RPiGpio"
pkgname="${_gitname}-git"
pkgver=0
pkgrel=1
pkgdesc="wrapper for sysfs access to raspberry pi gpio pins"
url="https://github.com/TrilbyWhite/RPiGpio"
arch=('armv6h')
license=('GPL3' 'CC-BY-SA')
makedepends=('git')
source=("${_gitname}::git://github.com/TrilbyWhite/RPiGpio.git")
sha256sums=('SKIP')

pkgver() {
	cd "${_gitname}";
	echo "0.$(git rev-list --count HEAD).$(git describe --always )"
}

build() {
	cd "${_gitname}"
	make
}

package() {
	cd "${_gitname}"
	make PREFIX=/usr DESTDIR="${pkgdir}" install
}
