# Install
To install follow the next steps:
1. open up a new cmd as administrator and run this command: npm install --global --production windows-build-tools --vs2015
2. npm config set msvs_version 2015 --global
3. close all instances of shell/cmd, reopen a cmd (regular this time, non-administrator) return to your directory where the project is located and run this command: npm run rebuild
4. npm start
