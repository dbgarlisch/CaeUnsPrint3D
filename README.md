# caeplugin-AMF
A Pointwise CAE plugin that exports a grid in the Additive Manufacturing File Format.

![AMF][AMFlogo]

Additive Manufacturing File Format (AMF) is an open standard for describing objects for additive manufacturing processes such as 3D printing. It is an XML-based format that describes the shape and composition of any 3D object to be fabricated on a 3D printer. Unlike the STL format, AMF has native support for color, materials, lattices, and constellations.

For more information see:
* [AMF wiki][AMFwiki]
* [AMF on wikipedia][AMFwikipedia]

## Building the Plugin
To build the AMF plugin you must integrate this source code into your local PluginSDK installation by following these steps.

* Download and install the [Pointwise Plugin SDK][SDKdownload].
* Configure and validate the SDK following the [SDK's instructions][SDKdocs].
* Create an AMF plugin project using the mkplugin script: `mkplugin -uns -cpp AMF`
* Replace the project's generated files with the files from this repository.


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

[AMFlogo]: https://raw.github.com/dbgarlisch/CaeUnsAMF/master/AMF_icon.png  "AMF Logo"
[AMFwiki]: http://amf.wikispaces.com/
[AMFwikipedia]: http://en.wikipedia.org/wiki/Additive_Manufacturing_File_Format
[SDKdocs]: http://www.pointwise.com/plugins
[SDKdownload]: http://www.pointwise.com/plugins/#sdk_downloads
