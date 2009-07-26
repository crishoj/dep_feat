# encoding: UTF-8

module Annotation
  class Animacy < Lemma

    def feature
      'anim'
    end

    def pre_corpus
      # Prepare lemmatization
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
      @stats = { :found => 0, :total => 0}
    end

    def mark_token
      # Lemmatize
      super
      # Only check nouns and pronouns for animacy
      if ['N', 'P'].include? @token.cpos
        # Animacy data lacks special characters and accented characters
        if is_animate?
          @token.features << feature
          @stats[:found] += 1
        elsif is_dead?
          @stats[:found] += 1
        else
          # Not found
          #puts "Not found: #{@token.lemma}"
        end
        @stats[:total] += 1
      end
      # Drop lemma again
      @token.lemma = nil
    end

    def post_corpus
      ratio = @stats[:found].to_f / @stats[:total].to_f
      puts sprintf("Animacy stats: %d/%d (%.2f%%) found",
        @stats[:found], @stats[:total], ratio*100)
    end

    protected

    def is_animate? 
      lookup @token, @animate
    end
    
    def is_dead?
      lookup @token, @dead
    end

    def lookup(token, list)
      list.include? lookup_key(token)
    end

    def lookup_key(token)
      token.lemma.gsub(/[æøåé]/i, '')
    end

    def parse_line(line)
      parts = line.split(/# /)
      [parts.first == '1', parts.last.gsub('=', ' ').gsub('+', '-').gsub(/�/, '').chomp]
    end

  end
end
