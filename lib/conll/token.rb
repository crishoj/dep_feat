
module Conll
  Token = Struct.new(:id, :form, :lemma, :cpos, :pos, :feats, :head_id, :deprel, :phead_id, :pdeprel)

  class Token
    attr_accessor :sentence

    def self.parse(line)
      fields = line.split(/\t/)
      fields = fields[0..2] + fields[3..-1].collect do |f|
        # Interpret dash/underscore as "missing value" and use nil instead
        (f == '_' or f == '-') ? nil : f
      end
      # Pass in a reference to the sentence
      Token.new(*fields)
    end

    def initialize(*vals)
      super(*vals)
      # Split features
      @features = vals[5].split(/\|/) unless vals[5].nil?
    end

    def features
      @features ||= []
    end

    def head
      find_token(self.head_id)
    end

    def phead
      find_token(self.phead_id)
    end

    def to_s
      if self.features.size > 0
        self.feats = self.features.join('|')
      else
        self.feats = nil
      end
      self.values.collect { |val|
        val.nil? ? '_' : val
      }.join("\t")
    end

    private

    def find_token(id)
      unless id == '0'
        @sentence.tokens[id.to_i - 1]
      end
    end

  end
end
