# encoding: UTF-8

module Annotation
  class Clause < Base
    BOUNDARIES = %w{, ; : - –}
    
    def pre_sentence
      @clause = 1
    end

    def mark_token
      if BOUNDARIES.include? @token.form
        # Count clause boundary
        @clause += 1 
      else
        # Mark clause number
        @token.features << "clause=#{@clause}"
      end
    end

  end
end
