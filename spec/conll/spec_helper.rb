
Spec::Runner.configure do |config|

  config.before(:all) do
    @corpus_filename = 'spec/conll/corpus.conll'
    @corpus_file = File.open(@corpus_filename, 'r')
    @token_line = @corpus_file.gets
  end

end