# encoding: UTF-8

module Annotation
  class Animacy < Lemma

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
            @animate.push lemma.downcase
          else
            @dead.push lemma.downcase
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
          mark_animate
          @stats[:found] += 1
        elsif is_dead?
          mark_dead
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

    def mark_animate
      @token.features << 'anim=1'
    end

    def mark_dead
      @token.features << 'anim=0'
    end

    def lookup(token, list)
      list.include? lookup_key(token)
    end

    def lookup_key(token)
      token.lemma.gsub(/[æøåé]/i, '').downcase
    end

    def parse_line(line)
      parts = line.split(/# /)
      [parts.first == '1', parts.last.gsub('=', ' ').gsub('+', '-').gsub(/�/, '').chomp]
    end

  end
end
