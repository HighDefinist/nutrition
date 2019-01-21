# projecttemplate
**A starting point for a project**

### Requirements
 
- C++11
- [CMake](https://cmake.org/)

### Installation

### Tested on

| [Linux][lin-link] | [Windows][win-link] | [Code-coverage][cov-link] |
| :---------------: | :---------------: | :---------------: |
| ![lin-badge]      | ![win-badge]      | ![cov-badge]      | 

[lin-badge]: https://travis-ci.org/HighDefinist/projecttemplate.svg?branch=master "Travis build status"
[lin-link]:  https://travis-ci.org/HighDefinist/projecttemplate "Travis build status"
[win-badge]: https://ci.appveyor.com/api/projects/status/qt756wkyja3ctio1/branch/master?svg=true "AppVeyor build status"
[win-link]:  https://ci.appveyor.com/project/HighDefinist/projecttemplate/branch/master "AppVeyor build status"
[cov-badge]: https://codecov.io/gh/HighDefinist/projecttemplate/branch/master/graph/badge.svg "Code coverage status"
[cov-link]:  https://codecov.io/gh/HighDefinist/projecttemplate/branch/master "Code coverage status"

- Visual Studio 2015 or newer
- GCC 5 or newer
- Clang 4 or newer
- XCode 6.4 or newer

### Download 

You can download the latest version of *templateproject* by cloning the GitHub repository:

	git clone https://github.com/HighDefinist/templateproject.git
	
### Usage

How to initialize the project:

* Download projecttemplate
* Remove .git directory
* *Optional: Rename project name in CMakeLists.txt to your own name $projectname$*
* Go to your GitHub account name $username$ and create a repository named $projectname$
* Open Git Bash in the project directory, and enter the following commands:
  * git init
  * git add .
  * git commit -m "Initial commit"
  * git remote add origin https://github.com/$username$/$projectname$.git
  * git push -u origin master

How to setup the project:
* Replace the contents of include/ and ext/ with whatever includes you need *(Note: these two folders are functionally identical, but generally include/ should contain your own files, and ext/ should contain external header-only libraries)*
* To setup a source executable named $exename$:
  * In src/CMakeLists.txt , modify SOURCE_LIST so that it contains $exename$
  * Create a folder named src/$exename$
  * Create a file named src/$exename$/$exename$.cpp (this is the main.cpp for the executable <exename>)
* To test your source files using Catch (only files in include/ can be tested - this feature is not complete at this time):
  * Edit test/main.cpp accordingly
* To disable tests, remove add_subdirectory(test) in CMakeLists.txt 
  
 
