# encoding: UTF-8

module Annotation
  class Animacy < Lemma

    def feature
      'anim'
    end

    def pre_corpus
      super
      # Tag animate
      @dictionary_file = 'vendor/anim/data10_all.results'
      @animate = []
      @dead    = []
      File.open(@dictionary_file) do |f|
        while (line = f.gets)
          animate, lemma = parse_line(line)
          if animate
            @animate.push lemma
          else
            @dead.push lemma
          end
        end
      end
    end

    def mark_token
      # Lemmatize
      super
      # Only check nouns and pronouns for animacy
      if ['N', 'P'].include? @token.cpos
        # Animacy data lacks special characters and accented characters
        lookup = @token.lemma.gsub(/[æøåé]/i, '')
        @token.features << feature if @animate.include? lookup
      end
      # Drop lemma again
      @token.lemma = nil
    end

    protected

    def parse_line(line)
      parts = line.split(/# /)
      [parts.first == '1', parts.last.gsub('=', ' ').gsub('+', '-').gsub(/�/, '').chomp]
    end

  end
end
