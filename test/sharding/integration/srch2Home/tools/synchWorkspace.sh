git push origin sharding-merge-master
currentDir=`pwd`
cd ~/workspace-srch2-v4/srch2-ngn/test/sharding;
git pull origin sharding-merge-master;
cd ~/workspace-srch2-1/srch2-ngn/test/sharding;
git pull origin sharding-merge-master;
cd ~/workspace-srch2-2/srch2-ngn/test/sharding;
git pull origin sharding-merge-master;
cd ~/workspace-srch2-3/srch2-ngn/test/sharding;
git pull origin sharding-merge-master;
cd $currentDir
# cd to tools path
sh ./compileWorkspaces.sh
cd $currentDir
