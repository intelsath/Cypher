const electron = require('electron');
const path = require('path');
const url = require('url');

const {app, globalShortcut, BrowserWindow, Menu} = electron;

let mainWindow;

// Listen for app to be ready
app.on('ready', function(){
    globalShortcut.register('CommandOrControl+Q', () =>{
        app.quit();
    });
    // Create new window
    mainWindow = new BrowserWindow({
        backgroundColor: '#4f5b69',
        'minHeight': 579,
        'minWidth': 800
    });
    // Erase menu
    //mainWindow.setMenu(null);
    // Load html into window
    mainWindow.loadURL(url.format({
        pathname: path.join(__dirname, 'mainWindow.html'),
        protocol: 'file',
        slashes: true
    }));

    // Developer Tools
    const mainMenuTemplate = [
        {
            label: 'Developer Tools',
            submenu:[
                {
                    label: 'Toggle DevTools',
                    accelerator: process.platform == 'darwin' ? 'Command+I' : 'Ctrl+I',
                    click(item, focusedWindow){
                        focusedWindow.toggleDevTools();
                    }
                },
                {
                    role: 'reload'
                }
            ]
        }
    ];

    // Build menu from template
    const mainMenu = Menu.buildFromTemplate(mainMenuTemplate);
    // Insert menu
    Menu.setApplicationMenu(mainMenu);
   
});
