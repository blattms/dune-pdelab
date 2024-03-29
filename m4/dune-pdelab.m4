# Additional checks needed to build the module
AC_DEFUN([DUNE_PDELAB_CHECKS],
[
  AC_REQUIRE([RVALUE_REFERENCES_CHECK])
  AC_REQUIRE([VARIADIC_TEMPLATES_CHECK])
  AC_REQUIRE([VARIADIC_CONSTRUCTOR_SFINAE_CHECK])
  AC_REQUIRE([DUNE_PATH_PETSC])
  AC_REQUIRE([DUNE_EIGEN])
  AC_REQUIRE([DUNE_FUNC_POSIX_CLOCK])
  DUNE_ADD_MODULE_DEPS([dune-pdelab], [POSIX_CLOCK],
    [$POSIX_CLOCK_CPPFLAGS], [$POSIX_CLOCK_LDFLAGS], [$POSIX_CLOCK_LIBS])
])

# Additional checks needed to find the module
AC_DEFUN([DUNE_PDELAB_CHECK_MODULE],[
  AC_MSG_NOTICE([Searching for dune-pdelab...])
  DUNE_CHECK_MODULES([dune-pdelab], [pdelab/common/clock.hh],[dnl
    return Dune::PDELab::getWallTime().tv_sec;
  ])
])
