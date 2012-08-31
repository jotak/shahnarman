cost = {0,1,0,0}
icon = "intensivefarming"
allowedInBattle = 0

function getName()
	if language == "french" then
		return "Culture intensive"
	else
		return "Intensive farming"
	end
end

function getDescription()
	if language == "french" then
		return "Augmente de 5 la nourriture sur une case."
	else
		return "Increase by 5 food production on a tile."
	end
end

function onCast()
	selectTarget("tile", "")
end

-- this function receives map coords. as parameter, in a space-separated string
function onResolve(params, caster)
	x, y = split(params, " ")
	attachToTile(x, y)
end

function getMod_food(value)
	return value + 5
end
