#
# Cookbook Name:: server-game
# Recipe:: default
#

# Install the needed packages
case node['platform']
when 'ubuntu'
	package 'libzmq-dev'
	package 'libprotobuf-dev'
	package 'protobuf-compiler'
	package 'cmake'
	package 'libboost-all-dev'
	package 'liblog4cxx10-dev'
end


# Clone server-game from git
git "/home/vagrant/orwell-int/server-game-master" do
	repository "git://github.com/orwell-int/server-game.git"
	reference "master"
	action :sync
end

