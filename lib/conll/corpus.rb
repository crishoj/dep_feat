
module Conll
  class Corpus
    attr_reader :sentences, :filename

    def self.parse(file)
      corpus = Corpus.new(file) do |corpus|
        File.new(file).each("\n\n") do |part|
          lines = part.split(/\n/)
          corpus << Conll::Sentence.parse(lines)
        end
      end
    end

    def <<(sentence)
      sentence.corpus = self
      sentence.index = @sentences.size
      @sentences << sentence
    end

    def initialize(filename = '')
      @filename  = filename
      @sentences = []
      yield self if block_given?
      self
    end

    def to_s
      @sentences.join("\n\n") + "\n"
    end

    def grep(options)
      form_re = Regexp.compile(options.form_re) if options.form_re
      for sentence in @sentences
        matched = false
        for token in sentence.tokens
          next if options.pos     and not token.pos == options.pos
          next if options.cpos    and not token.cpos == options.cpos
          next if options.form    and not token.form == options.form
          next if options.feat    and not token.features.include? options.feat
          next if options.form_re and not token.form =~ form_re
          matched = true
          break
        end
        yield sentence if matched
      end
    end
    
  end
end
