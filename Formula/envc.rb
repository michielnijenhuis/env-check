class Envc < Formula
    desc "A program that compares two .env files and shows their values and differences."
    homepage "https://github.com/michielnijenhuis/env-check"
    url "https://github.com/michielnijenhuis/env-check/archive/refs/tags/v1.2.3.tar.gz"
    sha256 "6531f47d2703b03e3b35802b4b45657e3e80a646133494ac45dcad368cadf13e"
    license "MIT"
    
    depends_on "make"

    def install
        system "make"
        bin.install "bin/envc"
    end
end