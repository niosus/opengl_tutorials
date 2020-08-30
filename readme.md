# 🚧 OpenGL tutorials 🚧 #

This is just a repo for learning OpenGL following the awesome course: https://learnopengl.com/

Be warned, while I am learning, there is a lot of code that will be changed and
there is a lot of wrong decisions that are going to be made. This is a very
Work In Progress 🚧 project, so expect things to change without notice.
Obviously, don't depend on it! 

It uses C++17 just for the fun of it.

### Warning ###
For now, because of this [issue](https://github.com/bazelbuild/bazel/issues/11554) you will need to create a symlink for python on Ubuntu 20.04:
```cmd
sudo ln -s /usr/bin/python3 /usr/bin/python 
```

## Approach ##
I try to follow the tutorials as they go, at the same time trying to outsource
the common functionality into libraries. For example, see the [gl/](gl/) folder
for the library that wraps usage of OpenGL, trying to protect the user from
making type mistakes. The idea is to make it readable and also as fast as
possible, i.e., using static memory allocation where possible, using traits to
make sure the sizes of types are guessed correctly and are translated into
OpenGL types properly.

## Credits ##
The underlying library is inspired by other efforts to achieve OOP approach to
OpenGL. Most notably a library of my ex-colleague Jens Behley:
https://github.com/jbehley/glow
