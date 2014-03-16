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

	# Do an apt-get update
	execute "apt-get-update-after" do
		command "apt-get update"
		ignore_failure true
		action :run
	end

	# Install needed packages
	package 'git'
	package 'build-essential'
	package 'libzmq-dev'
	package 'libprotobuf-dev'
	package 'protobuf-compiler'
	package 'cmake'
	package 'libboost-all-dev'
	package 'liblog4cxx10-dev'
	package 'gcc-4.8'
	package 'g++-4.8'

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

	# Clone server-game from git
	git "/home/vagrant/orwell-int/server-game" do
		repository "git://github.com/orwell-int/server-game.git"
		reference "master"
		user "vagrant"
		group "vagrant"
		enable_submodules true
		action :sync
	end

end


