class Envc < Formula
    desc "A program that compares two .env files and shows their values and differences."
    homepage "https://github.com/michielnijenhuis/env-check"
    url "https://github.com/michielnijenhuis/env-check/archive/refs/tags/v1.2.4.tar.gz"
    sha256 "33a8357ec0cf029c691567af905f53b958e4b7222b54f6d882dc8c583d194b6f"
    license "MIT"
    
    depends_on "make"

    def install
        system "make"
        bin.install "bin/envc"
    end
end