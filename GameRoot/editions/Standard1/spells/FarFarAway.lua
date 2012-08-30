cost = {0,4,0,0}
icon = "frozen_land"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Très très loin"
	else
		return "Far far away"
	end
end

function getDescription()
	if language == "french" then
		return "Augmente de 10 cases la portée des sorts."
	else
		return "Increase by 10 tiles the player's spells range."
	end
end

function onCast()
end

function onResolve(params, caster)
	attachToPlayer(caster)
end

function getMod_spells_range(val)
	return val + 10
end