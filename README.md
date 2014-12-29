## Remote - Control your iPhone from Xcode

"Remote" is a plugin for Xcode that allows you to control an iPhone 
from a window on your Mac during development. Originally created to avoid
having to pick  up a device during testing you can record "macros" 
of device touches and replay them comparing the resulting screen output 
against a snapshot for end-to-end testing. The Macro window is an 
editable WebView that can be modified at will.

![Icon](http://injectionforxcode.johnholdsworth.com/remote.png)

To use, download the zipped source for this plugin, build and restart 
Xcode. You can then patch your project's main.m to include the Remote 
client header using "Product/Remote/Patch App" or it can load from a 
bundle on the fly if using the simulator. When using a device check that
the correct IP address has been patched into main.m so the device can connect.

The display shadowing window will not display by default. Use the
Menu item "Product/Remote/Load" to have it appear. Thereafter, touches
on the shadow display or device applied and recorded. To save a macro 
or a sequence of touches, enter a name into the textfield towards the 
bottom of the touch display and click the save button. You can then 
replay the macro either by loading it using the pulldown menu 
at the top of the touch display and clicking replay or directly from 
the "Product/Remote/<Macro Name>" Menu.

For end-to-end testing, include a snapshot in a macro by clicking
the ""Snapshot" button. On replay, the macro will pause until the screen 
matches the snapshot within the specified tolerance or it will timeout 
asking if you would like to update the snapshot or the tolerance used. 
The units of tolerance are the number of bytes the screen image differs 
after the run length encoding of simular pixel values. 

### Implementation

- RMPluginController - interface between the Remote display and Xcode

- RMWindowController - macro re-player and overall nib controller

- RMMacroManager - controls display, saving and loading of macros

- RMImageView - subclass of NSImageView for event capture/device display

- RMDeviceController - interface between display and device

- RemoteCapture.h - #imported into application's main.m to connect to Xcode

### Limitations

Most activity on the device is captured including the keyboard. UIAlertView
prompts are not captured however which seem to render outside the window hierarchy.
Also, UIDatePickers are not rendered correctly although they will respond to events.

### Temporary License

Depending on interest this project may morph into a product so, for now,
the  Plugin is "licensed for download any use during iPhone development 
until the end of March 2015" and may not be redistributed other than 
through github. I've made the source available for as much of the code 
as possible should you want to change the interface but held back one 
source file in a precompiled library. Please file any bug reports
or fixes using github issues and pull requests if you encounter any problems.

### As ever:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
