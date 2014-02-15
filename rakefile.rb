class String
  def naturalized
    scan(/[^\d\.]+|[\d\.]+/).collect { |f| f.match(/\d+(\.\d+)?/) ? f.to_f : f }
  end
end

desc 'make engine'
task 'engine' do
    system %q("C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\IDE\devenv.com" Qatapult.sln /rebuild release)
end

desc 'make setup'
task :setup => [:inc_version] do
    version=File.read('version').strip
    timestamp=Time.now.strftime('%Y%m%d%H%M%S')
    File.open('workdir\\currentversion','w+') { |f| f.write(timestamp) }
    FileUtils.mkdir('setup\\'+version)
    system %q("C:\Program Files (x86)\Inno Setup 5\iscc.exe" "/dMyAppVersion=__VERSION__" /O"setup\__VERSION__" /cc Qatapult.iss).gsub(/__VERSION__/,version)
    File.open('setup\\currentversion','w+') { |f| f.write(timestamp) }
    FileUtils.cp("setup\\#{version}\\QatapultSetup.exe","QatapultSetup.exe")
end

require 'net/ssh'
require 'net/scp'
desc 'upload the latest release of qatapult and enable it'
task :upload do
    versions=Dir.glob('setup/*/')
    versions.delete('setup/old/')
    last_version=versions.sort_by! { |file| file.to_s.naturalized }.last

    Net::SCP.start('sd-27268.dedibox.fr', 'ecaradec') do |scp|
        scp.upload! last_version, '/var/www/emmanuelcaradec/qatapult/bin/', :recursive=>true
    end
    Net::SSH.start('sd-27268.dedibox.fr', 'ecaradec') do |ssh|
        ssh.exec "echo #{File.basename(last_version)} > /var/www/emmanuelcaradec/qatapult/bin/currentversion"
    end
end

desc 'rebuild everything in a releasable package'
task :release => [:engine, :setup]

task :inc_version do
    version=File.read('version')
    version=version.split('.')    
    version[2]=version[2].to_i+1
    version=version.join('.')
    File.write('version',version)
end
