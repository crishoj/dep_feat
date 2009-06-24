
module Conll
  FIELDS = [ :id, :form, :lemma, :cpos, :pos, :feats, :head_id, :deprel, :phead_id, :pdeprel ]
  Token = Struct.new(*FIELDS)

  class Token

    def self.parse(line)
      fields = line.split(/\t/).collect do |f|
        # Interpret underscore as "missing value" and use nil instead
        (f == '_') ? nil : f
      end
      # Split features
      fields[5] = fields[5].split(/\|/) unless fields[5].nil?
      # Pass in a reference to the sentence
      Token.new(*fields)
    end

    def sentence=(sentence)
      @sentence = sentence
      sentence.tokens << self
    end

    def head
      find_token(self.head_id)
    end

    def phead
      find_token(self.phead_id)
    end

    private

    def find_token(id)
      unless id == '0'
        @sentence.tokens[id.to_i - 1]
      end
    end

  end
end
