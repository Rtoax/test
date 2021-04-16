# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  config.vm.define "ubuntu", primary: true do |ubuntu_config|
    ubuntu_config.vm.box = "wholebits/ubuntu17.04-64"
    ubuntu_config.vm.network "private_network", ip: "192.168.10.100"

    ubuntu_config.vm.synced_folder ".", "/bpf-usdt-driver", type: "nfs"

    ubuntu_config.vm.provision "shell", path: "provision/install.sh", privileged: false
  end
end
