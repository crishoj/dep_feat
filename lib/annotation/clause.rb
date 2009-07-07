# encoding: UTF-8

module Annotation
  class Clause < Annotator
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

    def evaluate_token
      if dep_crossing?
        count_token(:crossing)
      else
        count_token(:non_crossing)
      end
    end

    def dep_crossing?
      clause = @gold_token.features.find { |f| f =~ /^clause=/ }
      system_head_idx = @token.head.index
      head = @gold_sentence.tokens[system_head_idx]
      head_clause = head.features.find { |f| f =~ /^clause=/ }
      if clause
        # Check if the head is in another clause
        head_clause != clause
      else
        # Check if the head is not in a clause
        not head_clause.nil?
      end
    end

  end
end
