from conans import ConanFile, CMake, tools


class DotenvConan(ConanFile):
    name = "dotenv"
    version = "0.1"
    requires = []
    license = "MIT"
    author = "<Put your name here> <And your email here>"
    url = "https://github.com/Fabio3rs/dotenv-cpp"
    description = "DotEnv implementation in C++17"
    topics = ("<Put some tag here>", "<here>", "<and here>")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "cmake"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        self.run("git clone https://github.com/Fabio3rs/dotenv-cpp.git")

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder="dotenv-cpp")
        cmake.build()

        # Explicit way:
        # self.run('cmake %s/dotenv-cpp %s'
        #          % (self.source_folder, cmake.command_line))
        # self.run("cmake --build . %s" % cmake.build_config)

    def package(self):
        self.copy("include/*.h", dst="include", src="dotenv-cpp", keep_path=False)
        self.copy("include/*.hpp", dst="include", src="dotenv-cpp", keep_path=False)
        self.copy("*dotenv_lib.lib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["dotenv_lib"]

