raise "Header file to mock must be specified!" unless ARGV.length >= 1
raise "Output directory must be specified!" unless ARGV.length >= 2

here = File.dirname __FILE__
require_relative  "#{here}/../lib/cmock"
cmock = CMock.new({:plugins => [:ignore, :return_thru_ptr, :ignore_arg, :expect_any_args, :array, :cexception, :callback, :return_thru_ptr], :mock_prefix => "Mock", :mock_path => ARGV[1]})
cmock.setup_mocks(ARGV[0])
