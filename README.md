# caeplugin-Print3D
A Pointwise CAE plugin that exports a grid suitable for 3D printing.

![Print3D Wing Image][WingImage]

The exported grid is converted to a collection if inflated edges. That is, each unique grid edge is exported as a cylinder. The cylinders are saved to an STL file.

Several solver attribute configuration settings are available to control export behavior.

Due to the limitations of 3D printing, only coarse grids can be successfully printed.

For more information see [Printing Grids in 3D][Print3Dblog] at the Pointwise blog.

## Building the Plugin
To build the Print3D plugin you must integrate this source code into your local PluginSDK installation by following these steps.

* Download and install the [Pointwise Plugin SDK][SDKdownload].
* Configure and validate the SDK following the [SDK's instructions][SDKdocs].
* Create an Print3D plugin project using the mkplugin script: `mkplugin -uns -cpp Print3D`
* Replace the project's generated files with the files from this repository.
* This plugin uses the Configurable Math Library. You can download it from the [CML website][CMLwebsite].


## Disclaimer
Plugins are freely provided. They are not supported products of
Pointwise, Inc. Some plugins have been written and contributed by third
parties outside of Pointwise's control.

TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, POINTWISE DISCLAIMS
ALL WARRANTIES, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED
TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE, WITH REGARD TO THESE SCRIPTS. TO THE MAXIMUM EXTENT PERMITTED
BY APPLICABLE LAW, IN NO EVENT SHALL POINTWISE BE LIABLE TO ANY PARTY
FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS
INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
INABILITY TO USE THESE SCRIPTS EVEN IF POINTWISE HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGES AND REGARDLESS OF THE FAULT OR NEGLIGENCE OF
POINTWISE.

[WingImage]: https://raw.github.com/dbgarlisch/CaeUnsPrint3D/master/print3d-wing-final.jpg  "Printed Wing Image"
[Print3Dblog]: http://blog.pointwise.com/2012/03/12/printing-grids-in-3d/
[SDKdocs]: http://www.pointwise.com/plugins
[SDKdownload]: http://www.pointwise.com/plugins/#sdk_downloads
[CMLwebsite]: http://cmldev.net/
