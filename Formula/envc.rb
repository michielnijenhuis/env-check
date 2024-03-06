class Envc < Formula
    desc "A program that compares two .env files and shows their values and differences."
    homepage "https://github.com/michielnijenhuis/env-check"
    url "https://example.com/my-cli/archive/v1.0.0.tar.gz"
    sha256 "..."
    license "MIT"
    
    depends_on "make"

    def install
        system "make"
        bin.install "bin/envc"
    end
end