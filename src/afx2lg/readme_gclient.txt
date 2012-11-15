This project uses GTest for testing and a .gclient file (see 'depot_tools') to download dependencies.
Below a basic .gclient file that's required to sync the relevant repositories:
---
solutions = [
  { "name"        : "src",
    "url"         : "https://afx2lg.googlecode.com/svn/trunk/src",
  },
  { "name"        : "gtest",
    "url"         : "http://googletest.googlecode.com/svn/trunk",
  },
]
