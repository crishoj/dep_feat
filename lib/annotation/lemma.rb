
module Annotation
  class Lemma < Base

    def initialize
      @bin       = "vendor/cstlemma/bin/cstlemma"
      @base      = 'models/lemma'
      @flex_file = "#{@base}/flexrules.UTF8"
      @dict_file = "#{@base}/dict.UTF8"
      @translation_file = "#{@base}/translation.DDT"
      @friends_file = "#{@base}/friends.UTF8"
    end

    def pre_corpus
      @input_file =  '/tmp/lemmatiser-input.txt'
      @output_file = '/tmp/lemmatiser-output.txt'
      @lemmas = []
      File.open(@input_file, 'w') do |f|
        for sent in @corpus.sentences
          f.puts sent.tokens.collect { |token|
             "#{token.form}/#{token.pos}"
          }.join(" ")
        end
      end
      `#{@bin} -L -eU -l -f #{@flex_file} -d #{@dict_file} -x #{@translation_file} -v #{@friends_file} -i #{@input_file} -o #{@output_file}`
      File.open(@output_file) do |f|
        while (line = f.gets)
          @lemmas.push line.split(/\t/)[1]
        end
      end
    end

    def mark_token
      @token.lemma = @lemmas.shift
    end

  end
end
