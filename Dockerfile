FROM ubuntu:18.04

## for apt to be noninteractive
ENV DEBIAN_FRONTEND noninteractive
ENV DEBCONF_NONINTERACTIVE_SEEN true

ENV ORWELL_DISABLE_TEST_BROADCAST 1

RUN mkdir -p /workdir
WORKDIR /workdir
COPY . /workdir
RUN cd /workdir
RUN ls
RUN pwd

# Install
RUN apt-get -qq update

## preseed and install tzdata
RUN echo "tzdata tzdata/Areas select Europe" > /tmp/preseed.txt; \
	echo "tzdata tzdata/Zones/Europe select Berlin" >> /tmp/preseed.txt; \
	debconf-set-selections /tmp/preseed.txt && \
	rm -f /etc/timezone && \
	rm -f /etc/localtime && \
	apt-get install -y tzdata

RUN apt-get install -y -qq cmake
RUN apt-get install -y -qq git
RUN apt-get install -y -qq libprotobuf-dev protobuf-compiler
RUN apt-get install -y -qq libboost-all-dev
RUN apt-get install -y -qq valgrind
RUN apt-get install -y -qq libzmq3-dev
RUN apt-get install -y -qq liblog4cxx10v5 liblog4cxx-dev
RUN apt-get install -y -qq python2.7
RUN apt-get install -y -qq lcov
RUN apt-get install -y -qq libopencv-dev
RUN apt-get install -y -qq gcovr
RUN mkdir -p build
WORKDIR /workdir/build
RUN cmake ..
RUN make
RUN ctest --output-on-failure
RUN if [ "$(type clang)" = "$(type $CXX)" ] ; then exit 0 ; fi
RUN ctest -D ExperimentalMemCheck --output-on-failure
RUN sed -n -e '/LEAK SUMMARY:/,+8p' -e '/^Test\|Testing:[ ]\|Test:[ ]/p' Testing/Temporary/MemoryChecker.*.log
RUN mkdir -p build_coverage
WORKDIR /workdir/build_coverage
RUN cmake .. -DORWELL_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
RUN make orwell_coverage
RUN gcovr . -r .. -p | sed "/Missing/{s/\(.*\)Missing/\1/p;h;s/^/\n/;:lineloop;N;:charloop;s/\(.*\)\n.\n\(.\).*/\1\2/;t exitcharloop;s/\n.\(.\+\)\n\(.\)/\2\n\1\n/;t charloop;:exitcharloop;s/\n.*//;p;s/.*//;G;b lineloop}" -n
WORKDIR /workdir/build
