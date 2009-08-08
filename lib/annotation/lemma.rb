
module Annotation
  class Lemma < Base

    def initialize
      @bin       = "vendor/cstlemma/bin/cstlemma"
      @base      = 'models/lemma'
      @flex_file = "#{@base}/flexrules"
      @dict_file = "#{@base}/dict"
      @translation_file = "#{@base}/translation.DDT"
      @friends_file = "#{@base}/friends"
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
      `recode UTF-8..ISO-8859-1 #{@input_file}`
      `#{@bin} -L -e1 -f #{@flex_file} -d #{@dict_file} -x #{@translation_file} -v #{@friends_file} -i #{@input_file} -o #{@output_file}`
      `recode ISO-8859-1..UTF-8 #{@output_file}`
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
