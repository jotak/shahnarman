cost = {0,1,4,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Cercle de magie"
	else
		return "Magic circle"
	end
end

function getDescription()
	if language == "french" then
		return "Place un cercle de magie sur la carte, afin de pouvoir y lancer des sorts."
	else
		return "Put a magic circle on the map, in order to cast spells from there."
	end
end

function onCast()
	selectTarget("tile", "")
end

function onResolve(params, caster)
	x, y = split(params, " ")
	addMagicCircle(caster, x, y)
end
