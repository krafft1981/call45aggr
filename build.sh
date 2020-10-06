#!/bin/bash

name=call45aggr

dockerImage() {
	docker build . --tag=$name
}

dockerBuild() {
	if [ ! -d ~/root ]
	then
		mkdir ~/root
	fi

	if [ -z "$(docker images | grep $name)" ]
	then
		./$0 docker image
	fi

	docker run -it --rm -v $(pwd):/home -v ~/root:/root $name ./build.sh build
	docker run -it --rm -v $(pwd):/home -v ~/root:/root $name ./build.sh rpm
}

dockerShell() {
	if [ -z "$(docker images | grep $name)" ]
	then
		./$0 docker image
	fi

	docker run -it --rm -v $(pwd):/home -v ~/root:/root -p 8080:8080 --cap-add SYS_ADMIN $name bash
}

dockerRun() {
	if [ -z "$(docker images | grep $name)" ]
	then
		./$0 docker image
	fi

	if [ ! -f bin/call45aggr ]
	then
		./$0 docker build
	fi

	docker run -td -v $(pwd):/home -v ~/root:/root -p 8080:8080 --cap-add SYS_ADMIN $name /home/bin/call45aggr /home/conf/call45aggr.json
}

dockerClean() {
	docker rm $(docker ps -a | awk '{print $1}') --force
	docker rmi $(docker images | awk '{print $3}') --force
}

build() {

scl enable devtoolset-7 bash << \EOF
printf "
[settings]
os_build=Linux
arch_build=x86_64
os=Linux
arch=x86_64
compiler=gcc
compiler.version=7
compiler.libcxx=libstdc++
" > profile

conan install . -pr profile --build bzip2 --build zlib  --build jsoncpp --build boost
EOF

	export CC=/opt/rh/devtoolset-7/root/usr/bin/gcc
	export CXX=/opt/rh/devtoolset-7/root/usr/bin/g++
	cmake3 CMakeLists.txt
	make -j $(nproc)
}

clean() {
	rm -f profile cmake_install.cmake compile_commands.json conanbuildinfo.* conaninfo.txt
	rm -rf CMakeCache.txt Makefile CMakeFiles conan.lock graph_info.json build
}

rpm() {
	echo "rpm build not ready"
}

if [ "$1" = "docker" ]
then
	case $2 in
		[Ii][Mm][Aa][Gg][Ee])
			dockerImage
		;;

		[Bb][Uu][Ii][Ll][Dd])
			dockerBuild
		;;

		[Ss][Hh][Ee][Ll][Ll])
			dockerShell
		;;

		[Rr][Uu][Nn])
			dockerRun
		;;

		[Aa][Ll][Ll])
			dockerImage
			dockerBuild
			dockerShell
		;;

		[Cc][Ll][Ee][Aa][RrNn])
			dockerClean
		;;

		*)
			echo "uncknown docker command: $2"
		;;
	esac
else
	case $1 in
		[Bb][Uu][Ii][Ll][Dd])
			build
		;;

		[Rr][Pp][Mm])
			rpm
		;;

		[Cc][Ll][Ee][Aa][NnRr])
			clean
		;;

		*)
			build
			rpm
		;;
	esac
fi
