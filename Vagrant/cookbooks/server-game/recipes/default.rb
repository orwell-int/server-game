#
# Cookbook Name:: server-game
# Recipe:: default
#

# If we are under Ubuntu
case node['platform']
when 'ubuntu'

	# Use the FR update site, which is faster than the US one
	execute "sed-us-to-fr" do
		command "sed -i 's/us/fr/g' /etc/apt/sources.list"
		ignore_failure true
		action :run
	end

	# Do an apt-get update
	execute "apt-get-update-before" do
		command "apt-get update"
		ignore_failure true
		action :run
	end

	package 'python-software-properties'

	# Install the APT for gcc 4.8
	execute "add-apt-ubuntu-test" do
		command "add-apt-repository ppa:ubuntu-toolchain-r/test -y"
		action :run
	end

	# Install the APT for the newest zmq's version
	execute "add-apt-ubuntu-test" do
		command "add-apt-repository ppa:chris-lea/zeromq -y"
		action :run
	end

	# Do an apt-get update
	execute "apt-get-update-after" do
		command "apt-get update"
		ignore_failure true
		action :run
	end

	# Install needed packages
	package 'git'
	package 'build-essential'
	package 'libzmq3-dbg'
	package 'libzmq3-dev'
	package 'libprotobuf-dev'
	package 'protobuf-compiler'
	package 'cmake'
	package 'libboost-all-dev'
	package 'liblog4cxx10-dev'
	package 'gcc-4.8'
	package 'g++-4.8'
	package 'subversion'
	package 'libgtest-dev'
	package 'python-virtualenv'

	# Update alternatives GCC
	execute "update-alternatives-gcc" do
		command "update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 10"
		action :run
	end

	# Update alternatives G++
	execute "update-alternatives-g++" do
		command "update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 10"
		action :run
	end

	# Create the enclosing directory
	directory "/home/vagrant/orwell-int" do
		owner "vagrant"
		group "vagrant"
		mode 00755
		action :create
	end

	# Create the build directory
	directory "/home/vagrant/orwell-int/build" do
		owner "vagrant"
		group "vagrant"
		mode 00755
		action :create
	end

	# Clone server-game from git
	git "/home/vagrant/orwell-int/server-game" do
		repository   "git://github.com/orwell-int/server-game.git"
		destination  "/home/vagrant/orwell-int/server-game"
		reference    "master"
		user         "vagrant"
		group        "vagrant"
		enable_submodules true
		action :sync
	end

	# Clone googlemock
	subversion "svn-googlemock" do
		repository     "http://googlemock.googlecode.com/svn/trunk/"
		revision       "HEAD"
		destination    "/home/vagrant/googlemock"
		user           "vagrant"
		group          "vagrant"
		action :sync
	end

	# Prepare google mock
	execute "prepare-googlemock" do
		cwd "/home/vagrant/googlemock"
		command "cmake -DCMAKE_BUILD_TYPE=Release"
		user "vagrant"
		group "vagrant"
		action :run
	end

	# Compile google mock
	execute "compile-googlemock" do
		cwd "/home/vagrant/googlemock"
		command "make"
		user "vagrant"
		group "vagrant"
		action :run
	end
end

