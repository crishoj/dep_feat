
module Annotation
  class NumericalBullet < Bullet

    def list_counter?(token)
      token.form =~ /^[0-9]+$/ # A numeral
    end

  end
end
