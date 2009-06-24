
require 'lib/conll/token'

module Conll
  class Sentence
    attr_reader :tokens

    def self.parse(lines)
      sentence = Sentence.new
      lines.each do |line|
        token = Token.parse(line)
        token.sentence = sentence
      end
      sentence
    end

    def initialize(tokens = [])
      @tokens = tokens
    end
  end
end
